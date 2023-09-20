static:
	bazel build //examples/cpp/static:server
	bazel build //examples/cpp/static:client
socket:
	bazel build //examples/cpp/socket:server
	bazel build //examples/cpp/socket:client

.PHONY: static socket