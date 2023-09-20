static:
	bazel build //examples/cpp/static:server
	bazel build //examples/cpp/static:client

.PHONY: static