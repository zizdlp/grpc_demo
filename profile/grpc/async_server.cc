#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

class GreeterServiceImpl final : public Greeter::Service {
public:
  grpc::Status SayHello(::grpc::ServerContext* context, const ::helloworld::HelloRequest* request, ::helloworld::HelloReply* response) override {
    std::string prefix("Hello ");
    response->set_message(prefix + request->name());
    return grpc::Status::OK;
  }
};

class AsyncGreeterServiceImpl final : public Greeter::AsyncService {
public:
  class CallData {
  public:
    CallData(Greeter::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        new CallData(service_, cq_);
        std::string prefix("Hello ");
        response_.set_message(prefix + request_.name());

        status_ = FINISH;
        responder_.Finish(response_, grpc::Status::OK, this);
      } else {
        GPR_ASSERT(status_ == FINISH);
        delete this;
      }
    }

  private:
    Greeter::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    HelloRequest request_;
    HelloReply response_;
    grpc::ServerAsyncResponseWriter<HelloReply> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
  };
};

void RunServer(uint16_t port) {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

  GreeterServiceImpl sync_service;
  AsyncGreeterServiceImpl async_service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Register synchronous service
  // builder.RegisterService(&sync_service);

  // Register asynchronous service
  builder.RegisterService(&async_service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  RunServer(absl::GetFlag(FLAGS_port));
  return 0;
}
