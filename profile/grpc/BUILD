# Copyright 2020 the gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])

cc_binary(
    name = "client",
    srcs = ["client.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
    linkshared = False,
    linkstatic = True,

)
cc_binary(
    name = "client_multi",
    srcs = ["client_multi.cc","thread_pool.hpp"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
    linkshared = False,
    linkstatic = True,

)


cc_binary(
    name = "server",
    srcs = ["server.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings:str_format",
    ],
    linkshared = False,
    linkstatic = True,
)
cc_binary(
    name = "async_server",
    srcs = ["async_server.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings:str_format",
    ],
    linkshared = False,
    linkstatic = True,
)

cc_binary(
    name = "async_client",
    srcs = ["async_client.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
    linkshared = False,
    linkstatic = True,

)


cc_binary(
    name = "grpc_async_client",
    srcs = ["greeter_async_client.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
    linkshared = False,
    linkstatic = True,

)

cc_binary(
    name = "grpc_async_server",
    srcs = ["greeter_async_server.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//examples/protos:helloworld_cc_grpc",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings:str_format",
    ],
    linkshared = False,
    linkstatic = True,
)
