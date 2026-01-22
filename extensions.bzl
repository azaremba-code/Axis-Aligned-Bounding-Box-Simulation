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

argparse_extension = module_extension(
    implementation = _argparse_extension_impl,
)
