workspace(name = "axis_aligned_bb_sim")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# argparse library for C++
http_archive(
    name = "argparse",
    url = "https://github.com/p-ranav/argparse/archive/refs/tags/v3.2.tar.gz",
    strip_prefix = "argparse-3.2",
    build_file_content = """
cc_library(
    name = "argparse",
    hdrs = ["include/argparse/argparse.hpp"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
)