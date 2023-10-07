# https://github.com/yhirose/cpp-httplib
# commit v0.12.0
package(
    default_visibility = ["//visibility:public"],
    features = ["treat_warnings_as_errors"],
)

cc_library(
    name = "httplib",
    srcs = [
        "httplib.cc",
    ],
    hdrs = [
        "httplib.h",
    ],
    deps = [
        "@boringssl//:crypto",
    ],
)

