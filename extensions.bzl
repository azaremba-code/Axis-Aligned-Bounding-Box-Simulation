"""Module extension for defining external dependencies."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _argparse_extension_impl(module_ctx):
    http_archive(
        name = "argparse",
        url = "https://github.com/p-ranav/argparse/archive/refs/tags/v3.2.tar.gz",
        strip_prefix = "argparse-3.2",
        build_file_content = """
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "argparse",
    hdrs = ["include/argparse/argparse.hpp"],
    includes = ["include"],
    visibility = ["//visibility:public"],
)
""",
    )

def _highway_extension_impl(module_ctx):
    http_archive(
        name = "highway",
        url = "https://github.com/google/highway/archive/refs/tags/1.2.0.tar.gz",
        strip_prefix = "highway-1.2.0",
        build_file_content = """
load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "hwy",
    hdrs = glob(["hwy/**/*.h"]),
    includes = ["."],
    copts = [
        "-DHWY_STATIC_DEFINE",
        "-std=c++17",
    ],
    visibility = ["//visibility:public"],
)
""",
    )

argparse_extension = module_extension(
    implementation = _argparse_extension_impl,
)

highway_extension = module_extension(
    implementation = _highway_extension_impl,
)
