load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = [
        "main.cpp",
        "Simulation.h",
    ],
    deps = [
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
