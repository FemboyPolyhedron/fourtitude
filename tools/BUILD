exports_files(["stylecheck.py"])

load("@rules_python//python:defs.bzl", "py_test")

py_test(
    name = "stylecheck",
    srcs = ["stylecheck.py"],
    main = "stylecheck.py",
    args = ["src"],
    data = [
        "//src:all_srcs",
    ],
)