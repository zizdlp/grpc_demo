cc_binary(
    name = "client",
    srcs = ["client.cc"],
    defines = ["BAZEL_BUILD"],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//profile/http:httplib",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ],
    linkopts = ["-lssl", "-lcrypto"],  # 添加 OpenSSL 的链接选项
    linkshared = False,
    linkstatic = True,

)