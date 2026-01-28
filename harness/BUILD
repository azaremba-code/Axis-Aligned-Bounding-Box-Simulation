load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = [
        "main.cpp",
        
        "SimulationAdrian1.h",

        "SimulationEugene1.h",
        "SimulationEugene2.h",
        "SimulationEugene3.h",
        "SimulationEugene4.h",
        "SimulationEugene5.h",
    ],
    deps = [
        "//include:common",
        "//include:simulation",
        "@argparse",
        "@highway//:hwy",
    ],
    copts = [
        "-std=c++20",
        "-Wall",
        "-Wextra",
    ],
    visibility = ["//visibility:public"],
)
