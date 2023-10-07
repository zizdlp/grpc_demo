static:
	bazel build //examples/cpp/static:server
	bazel build //examples/cpp/static:client
grpc:
	bazel build //profile/grpc:server
	bazel build //profile/grpc:grpc_async_client
	bazel build //profile/grpc:grpc_async_server
	bazel build //profile/grpc:async_server
	bazel build //profile/grpc:async_client
	bazel build //profile/grpc:client
	bazel build //profile/grpc:client_multi
http:
	bazel build //profile/httplib:client
socket:
	bazel build //profile/socket:server
	bazel build //profile/socket:client
socket_server:
	bazel run //profile/socket:server
socket_client:
	bazel run //profile/socket:client
.PHONY: static socket