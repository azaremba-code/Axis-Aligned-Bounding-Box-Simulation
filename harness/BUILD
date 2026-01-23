load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = [
        "main.cpp",
        "SimulationAdrian1.h",
        "SimulationEugene1.h",
    ],
    deps = [
        "//include:common",
        "//include:simulation",
        "@argparse",
    ],
    copts = [
        "-std=c++20",
        "-Wall",
        "-Wextra",
    ],
    visibility = ["//visibility:public"],
)
