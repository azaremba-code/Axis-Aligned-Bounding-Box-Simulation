load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "common",
    hdrs = ["common/Timer.h", 
            "common/Concurrency.h"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "simulation",
    hdrs = [
        "simulation/ISimulation.h",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "rng",
    hdrs = [
        "rng/Xorshift128PlusSSE2.h",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)