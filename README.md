# a minimal grpc c++ demo with bazel

> **主要内容:** grpc c++ with bazel

## 测试

1. hello wolrd case
    `bazel build examples/cpp/helloworld:all`
2. streaming large data case (grpc memory usage continues to increase)
    `bazel build examples/cpp/streaming:all`
3. streaming large data case but Scheduled restart server
    `bazel build examples/cpp/restart_server:all`
## 注意事项

1. workspace 添加依赖
    - rules_proto
    - rules_proto_grpc
    - com_github_grpc_grpc

2. bazelrc 设置编译条件
    `build --cxxopt='-std=c++14'`


## 如何安装bazel

1. 进入[grpc release](https://github.com/bazelbuild/bazel/releases)下载电脑对应版本的bazel-*-installer-*.sh

2. 执行如下命令
    ```shell
    sudo bash bazel-*-installer-*.sh #电脑对应的安装版本文件
    export PATH="$PATH:$HOME/bin" # 根据提示修改为对应的安装位置
    bazel --version # 查看版本
    ```

## proxy

```shell
export https_proxy="http://127.0.0.1:1080"
export http_proxy="http://127.0.0.1:1080"
```