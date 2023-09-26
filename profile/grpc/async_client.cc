#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");
ABSL_FLAG(uint32_t, loop, 1000, "client call loop times");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreeterClient {
public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  void SayHelloAsync(const std::string& user, std::function<void(std::string)> callback) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    std::unique_ptr<grpc::ClientAsyncResponseReader<HelloReply>> rpc(
        stub_->PrepareAsyncSayHello(&context, request, &cq_));
    
    rpc->StartCall();
    rpc->Finish(&reply, &status_, (void*)1);
    
    void* got_tag;
    bool ok = false;

    GPR_ASSERT(cq_.Next(&got_tag, &ok));
    std::cout<<"client in call"<<std::endl;
    if (status_.ok()) {
      callback(reply.message());
    } else {
      std::cout << status_.error_code() << ": " << status_.error_message()
                << std::endl;
      callback("RPC failed");
    }
  }
  grpc::CompletionQueue cq_;
private:
  std::unique_ptr<Greeter::Stub> stub_;
  
  grpc::Status status_;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  GreeterClient greeter(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  
  int length=25000;
  std::string send_data(length, 'a');
  auto s= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
  .count();
  auto loop = absl::GetFlag(FLAGS_loop);
  std::string user(send_data);
  for(int i=0;i<loop;++i){
    std::cout<<"client call SayHelloAsync"<<std::endl;
    greeter.SayHelloAsync(user, [i](std::string reply) {
      std::cout << "Greeter received: " << reply << " for loop " << i << std::endl;
    });
  }
  auto e= std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
  .count();
  std::cout<<"loop:"<<loop <<" grpc time consume:"<<e-s<<"us"<<std::endl;

  // Run the event loop
  for (int i = 0; i < loop; ++i) {
    void* got_tag;
    bool ok = false;
    GPR_ASSERT(greeter.cq_.Next(&got_tag, &ok));
    GPR_ASSERT(ok);
    GPR_ASSERT(got_tag == (void*)1);
  }

  return 0;
}
