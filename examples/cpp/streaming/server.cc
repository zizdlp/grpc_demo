/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/data.grpc.pb.h"
#else
#include "data.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::ServerReaderWriter;
using data::GRPCDemo;
using data::Request;
using data::Response;
using namespace std;
// Logic and data behind the server's behavior.
class GRPCDemoServiceImpl final : public GRPCDemo::Service {
  Status StreamingMethod(ServerContext* context, ServerReaderWriter<Response,Request>* stream) override {
    Request req;
    int id=0;
    double total_length=0;
    while (stream->Read(&req)) {
      Response reply;
      std::string string_data (req.data());
      //reply.set_data(string_data);
      reply.set_data("");
      stream->Write(reply);
      total_length+=string_data.length();
      id++;
    }
    //cout<<"=== server streaming recv&send:"<<total_length/1024/1024 <<" MB"<<endl;
    return Status::OK;
  }

  Status UnaryMethod(ServerContext* context, const Request *request,Response *reply) override {
    //reply->set_data(request->data());
    reply->set_data("");
    //cout<<"=== server unary recv&send:"<<reply->data().size()/1024/1024 <<" MB"<<endl;
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  GRPCDemoServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.SetMaxReceiveMessageSize(-1);  // mydebug server set max message size;
  builder.SetMaxSendMessageSize(-1);
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
