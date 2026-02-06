load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_cuda//cuda:defs.bzl", "cuda_library")

cuda_library(
    name = "cuda_sim",
    srcs = [
        "cuda_simulation_eugene1.cu",
    ],
    hdrs = [
        "cuda_simulation_eugene1.cuh",
    ],
    deps = [
        "@cuda//:curand",
        "@cuda//:thrust",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "main",
    srcs = [
        "main.cpp",
    ],
    deps = [
        ":cuda_sim",
        "//include:common",
        "@argparse",
    ],
    copts = [
        "-std=c++20",
        "-Wall",
        "-Wextra",
    ],
    visibility = ["//visibility:public"],
)
