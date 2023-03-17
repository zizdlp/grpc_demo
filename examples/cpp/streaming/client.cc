
#include <random>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "stdlib.h"
#include "thread_pool.hpp"

#ifdef BAZEL_BUILD
#include "examples/protos/data.grpc.pb.h"
#else
#include "data.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using data::GRPCDemo;
using data::Request;
using data::Response;

uint64_t GetTimeStamp() {  // 直接调用此函数就可以返回时间戳了
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
      .count();
}
class GRPCDemoClient {
 public:
  GRPCDemoClient(std::shared_ptr<Channel> channel)
      : stub_(GRPCDemo::NewStub(channel)) {}


  std::string StreamingMethod(int length,char * data) {
    ClientContext context;

    std::shared_ptr<ClientReaderWriter<Request, Response> > stream(
        stub_->StreamingMethod(&context));
    std::thread writer([&,stream]() {
      std::vector<Request> requests;
      int left=0;
      int maxlength=1024*1024*3;
      int right=length<maxlength?length:maxlength;
      int id=0;
      while(left<length){
        std::string string_data (data+left, data+right);
        Request req;
        req.set_data(string_data);
        stream->Write(req);
        left=right;
        right=length<left+maxlength?length:left+maxlength;
        id++;
      }
      stream->WritesDone();
      //std::cout << "===streaming client send:"<<(double)length/1024/1024<<" MB" << std::endl;
    });

    Response server_reply;
    double recv_length=0;
    while (stream->Read(&server_reply)) {
      recv_length+=server_reply.data().length();
    }
    writer.join();
    //std::cout << "===streaming client recv::"<<recv_length/1024/1024<<" MB" << std::endl;
    Status status = stream->Finish();
    if (!status.ok()) {
      std::cout << "stream rpc failed." << std::endl;
    }
    return "stream end\n";
  }


  std::string UnaryMethod(int length,char * data) {

    Request request;
    request.set_data(data, length);
    Response reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    //std::cout << "===unary client send:"<<(double)length/1024/1024<<" MB" << std::endl;
    Status status = stub_->UnaryMethod(&context, request, &reply);
    //std::cout << "===unary client recv:"<<reply.data().size()/1024/1024<<" MB" << std::endl;
    return "stream end\n";
  }


 private:
  std::unique_ptr<GRPCDemo::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        target_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --target="
                  << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  } else {
    target_str = "localhost:50051";
  }
  thread_pool pool(5);
  grpc::ChannelArguments ch_args;  // mydebug grpc max message;
  ch_args.SetMaxReceiveMessageSize(-1);
  ch_args.SetMaxSendMessageSize(-1);
  auto  channel = grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), ch_args);
  GRPCDemoClient GRPCDemo(channel);

  while(true){
    for (int i=0;i<20;++i){
        pool.push_task([target_str,&GRPCDemo]{
          int length=100*1024*1024;//100MB
          char * data =new char[length];
          std::string reply = GRPCDemo.StreamingMethod(length,data);
          delete []data;
      },i%5);
  }
  pool.wait_for_tasks();
  }
  return 0;
}
