#pragma once
/**
 * @file thread_pool.hpp
 * @author Barak Shoshany (baraksh@gmail.com) (http://baraksh.com)
 * @version 2.0.0
 * @date 2021-08-14
 * @copyright Copyright (c) 2021 Barak Shoshany. Licensed under the MIT license.
 * If you use this library in published research, please cite it as follows:
 *  - Barak Shoshany, "A C++17 Thread Pool for High-Performance Scientific
 * Computing", doi:10.5281/zenodo.4742687, arXiv:2105.00613 (May 2021)
 *
 * @brief A C++17 thread pool for high-performance scientific computing.
 * @details A modern C++17-compatible thread pool implementation, built from
 * scratch with high-performance scientific computing in mind. The thread pool
 * is implemented as a single lightweight and self-contained class, and does not
 * have any dependencies other than the C++17 standard library, thus allowing a
 * great degree of portability. In particular, this implementation does not
 * utilize OpenMP or any other high-level multithreading APIs, and thus gives
 * the programmer precise low-level control over the details of the
 * parallelization, which permits more robust optimizations. The thread pool was
 * extensively tested on both AMD and Intel CPUs with up to 40 cores and 80
 * threads. Other features include automatic generation of futures and easy
 * parallelization of loops. Two helper classes enable synchronizing printing to
 * an output stream by different threads and measuring execution time for
 * benchmarking purposes. Please visit the GitHub repository at
 * https://github.com/bshoshany/thread-pool for documentation and updates, or to
 * submit feature requests and bug reports.
 */

#define THREAD_POOL_VERSION "v2.0.0 (2021-08-14)"
#include <unistd.h>
#include <atomic>       // std::atomic
#include <chrono>       // std::chrono
#include <cstdint>      // std::int_fast64_t, std::uint_fast32_t
#include <functional>   // std::function
#include <future>       // std::future, std::promise
#include <iostream>     // std::cout, std::ostream
#include <memory>       // std::shared_ptr, std::unique_ptr
#include <mutex>        // std::mutex, std::scoped_lock
#include <queue>        // std::queue
#include <thread>       // std::this_thread, std::thread
#include <type_traits>  // std::common_type_t, std::decay_t, std::enable_if_t, std::is_void_v, std::invoke_result_t
#include <utility>      // std::move
#include <vector>

/**
 * @brief
 * 线程池结构：1.每一个线程对应一个队列，对每一个队列只有两个线程操作：主线程（唯一）推任务给队列，任务线程弹出任务执行；
 * 2.推入任务无锁，弹出任务加锁是为了避免工作线程 while循环 占用资源，每弹入一个任务notify。
 * 3. wait 任务结束有另一个cv&mutex 对。
 * worker_conditions:主线程notify 任务线程
 * wait_condition:任务线程notify 主线程
 *
 * 在没有任务执行时：while loop 模式会占用一定的cpu资源，sleep间隔为100ms时20%，1000ms时3%左右。
 * 改用cv 模式，无任务执行时，cpu占用基本为0。此时相对于单线程耗时的一个瓶颈是push_task的锁等待。
 */

class thread_pool {
  typedef std::uint_fast32_t ui32;
  typedef std::uint_fast64_t ui64;

 public:
  // ============================
  // Constructors and destructors
  // ============================

  /**
   * @brief Construct a new thread pool.
   *
   * @param _thread_count The number of threads to use. The default value is the
   * total number of hardware threads available, as reported by the
   * implementation. With a hyperthreaded CPU, this will be twice the number of
   * CPU cores. If the argument is zero, the default value will be used instead.
   */
  explicit thread_pool(const ui32 &_thread_count = std::thread::hardware_concurrency())
      : thread_count(_thread_count), threads(new std::thread[_thread_count]) {
    create_threads();
  }

  /**
   * @brief Destruct the thread pool. Waits for all tasks to complete, then
   * destroys all threads. Note that if the variable paused is set to true, then
   * any tasks still in the queue will never be executed.
   */
  ~thread_pool() {
    wait_for_tasks();
    running = false;
    for (ui32 index = 0; index < thread_count; ++index) {
      auto condition = worker_conditions[index].get();
      condition->notify_one();
    }
    destroy_threads();
  }

  // =======================
  // Public member functions
  // =======================

  /**
   * @brief Push a function with no arguments or return value into the task
   * queue.
   *
   * @tparam F The type of the function.
   * @param task The function to push.
   */
  template <typename F>
  void push_task(const F &task, int i = 0) {
    tasks_total++;

    {
      std::unique_lock<std::mutex> lock(*worker_mutexs[i].get());
      tasks[i].push(std::function<void()>(task));
      worker_conditions[i].get()->notify_one();
    }
  }
  /**
   * @brief Push a function with return value into the task
   * queue.
   *
   * @tparam F The type of the function.
   * @param task The function to push.
   * @param i The thread use for task;
   */
  template <class F>
  auto push_task(F &&f, int i = 0) -> std::future<typename std::invoke_result<F>::type> {
    using return_type = typename std::invoke_result<F>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(f);
    std::future<return_type> res = task->get_future();
    tasks_total++;
    {
      std::unique_lock<std::mutex> lock(*worker_mutexs[i].get());
      tasks[i].push([task]() { (*task)(); });
      worker_conditions[i].get()->notify_one();
    }
    return res;
  }
  /**
   * @brief Push a function with arguments, but no return value, into the task
   * queue.
   * @details The function is wrapped inside a lambda in order to hide the
   * arguments, as the tasks in the queue must be of type std::function<void()>,
   * so they cannot have any arguments or return value. If no arguments are
   * provided, the other overload will be used, in order to avoid the (slight)
   * overhead of using a lambda.
   *
   * @tparam F The type of the function.
   * @tparam A The types of the arguments.
   * @param task The function to push.
   * @param args The arguments to pass to the function.
   */
  template <typename F, typename... A>
  void push_task(const F &task, const A &...args) {
    push_task([task, args...] { task(args...); });
  }

  /**
   * @brief Wait for tasks to be completed. Normally, this function waits for
   * all tasks, both those that are currently running in the threads and those
   * that are still waiting in the queue. However, if the variable paused is set
   * to true, this function only waits for the currently running tasks
   * (otherwise it would wait forever). To wait for a specific task, use
   * submit() instead, and call the wait() member function of the generated
   * future.
   */
  void wait_for_tasks() {
    while (true) {
      std::unique_lock<std::mutex> lock(wait_mutex);
      wait_condition.wait(lock, [this] { return !this->running || tasks_total == 0; });
      if (tasks_total == 0) {
        break;
      }
    }
  }
  /**
   * @brief Get the number of threads in the pool.
   *
   * @return The number of threads.
   */
  ui32 get_thread_count() const { return thread_count; }

 private:
  // ========================
  // Private member functions
  // ========================

  /**
   * @brief Create the threads in the pool and assign a worker to each thread.
   */
  void create_threads() {
    tasks.resize(thread_count);
    worker_conditions.resize(thread_count);
    worker_mutexs.resize(thread_count);
    for (ui32 i = 0; i < thread_count; ++i) {
      worker_conditions[i] = std::make_unique<std::condition_variable>();
      worker_mutexs[i] = std::make_unique<std::mutex>();
      threads[i] = std::thread(&thread_pool::worker, this, i);
    }
  }

  /**
   * @brief Destroy the threads in the pool by joining them.
   */
  void destroy_threads() {
    for (ui32 i = 0; i < thread_count; i++) {
      threads[i].join();
    }
  }

  /**
   * @brief A worker function to be assigned to each thread in the pool.
   * Continuously pops tasks out of the queue and executes them, as long as the
   * atomic variable running is set to true.
   */
  void worker(int thread_id) {
    while (running) {
      std::function<void()> task;
      {
        std::unique_lock<std::mutex> lock(*worker_mutexs[thread_id].get());
        auto condition = worker_conditions[thread_id].get();
        condition->wait(lock, [this, thread_id] { return !this->running || !this->tasks[thread_id].empty(); });
        if (!running && this->tasks[thread_id].empty()) {
          return;
        }
        task = std::move(tasks[thread_id].front());
        this->tasks[thread_id].pop();
      }
      task();         // this shouled be in parallel
      tasks_total--;  // atomic
      {
        std::unique_lock<std::mutex> lock(wait_mutex);
        wait_condition.notify_one();
      }
    }
  }

  // ============
  // Private data
  // ============

  /**
   * @brief worker_mutexs and worker_conditions protect one queue read and write from same time;
   *
   */
  std::vector<std::unique_ptr<std::condition_variable>> worker_conditions;  // 主线程notify 任务线程
  std::vector<std::unique_ptr<std::mutex>> worker_mutexs;
  std::condition_variable wait_condition;  // 任务线程notify 主线程
  std::mutex wait_mutex;
  /**
   * @brief An atomic variable indicating to the workers to keep running. When
   * set to false, the workers permanently stop working.
   */
  std::atomic<bool> running = true;

  /**
   * @brief A queue of tasks to be executed by the threads.
   */
  std::vector<std::queue<std::function<void()>>> tasks;

  /**
   * @brief The number of threads in the pool.
   */
  ui32 thread_count;

  /**
   * @brief A smart pointer to manage the memory allocated for the threads.
   */
  std::unique_ptr<std::thread[]> threads;

  /**
   * @brief An atomic variable to keep track of the total number of unfinished
   * tasks - either still in the queue, or running in a thread.
   */
  std::atomic<ui32> tasks_total{0};
};

//                                     End class thread_pool //
// =============================================================================================
// //

// =============================================================================================
// //
//                                   Begin class synced_stream //

/**
 * @brief A helper class to synchronize printing to an output stream by
 * different threads.
 */
class synced_stream {
 public:
  /**
   * @brief Construct a new synced stream.
   *
   * @param _out_stream The output stream to print to. The default value is
   * std::cout.
   */
  explicit synced_stream(std::ostream &_out_stream = std::cout) : out_stream(_out_stream) {}

  /**
   * @brief Print any number of items into the output stream. Ensures that no
   * other threads print to this stream simultaneously, as long as they all
   * exclusively use this synced_stream object to print.
   *
   * @tparam T The types of the items
   * @param items The items to print.
   */
  template <typename... T>
  void print(const T &...items) {
    const std::scoped_lock lock(stream_mutex);
    (out_stream << ... << items);
  }

  /**
   * @brief Print any number of items into the output stream, followed by a
   * newline character. Ensures that no other threads print to this stream
   * simultaneously, as long as they all exclusively use this synced_stream
   * object to print.
   *
   * @tparam T The types of the items
   * @param items The items to print.
   */
  template <typename... T>
  void println(const T &...items) {
    print(items..., '\n');
  }

 private:
  /**
   * @brief A mutex to synchronize printing.
   */
  mutable std::mutex stream_mutex = {};

  /**
   * @brief The output stream to print to.
   */
  std::ostream &out_stream;
};

//                                    End class synced_stream //
// =============================================================================================
// //

// =============================================================================================
// //
//                                       Begin class timer //

/**
 * @brief A helper class to measure execution time for benchmarking purposes.
 */
class timer {
  typedef std::int_fast64_t i64;

 public:
  /**
   * @brief Start (or restart) measuring time.
   */
  void start() { start_time = std::chrono::steady_clock::now(); }

  /**
   * @brief Stop measuring time and store the elapsed time since start().
   */
  void stop() { elapsed_time = std::chrono::steady_clock::now() - start_time; }

  /**
   * @brief Get the number of milliseconds that have elapsed between start() and
   * stop().
   *
   * @return The number of milliseconds.
   */
  i64 ms() const { return (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time)).count(); }

 private:
  /**
   * @brief The time point when measuring started.
   */
  std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

  /**
   * @brief The duration that has elapsed between start() and stop().
   */
  std::chrono::duration<double> elapsed_time = std::chrono::duration<double>::zero();
};

//                                        End class timer //
// =============================================================================================
// //
