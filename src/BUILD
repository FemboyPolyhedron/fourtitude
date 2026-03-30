load("@rules_cc//cc:defs.bzl", "cc_library")

filegroup(
    name = "all_srcs",
    srcs = glob([
        "**/*.c",
        "**/*.h",
        "**/*.cpp",
        "**/*.hpp",
    ], allow_empty = True),
    visibility = ["//visibility:public"], 
)

cc_library(
    name = "fourtitude_c",
    srcs = [
        "fourtitude/Fourtitude.c",
        "fourtitude/FourtitudeElement.c",
        "fourtitude/FourtitudeElements.c",
        "fourtitude/FourtitudeEventSystem.c",
        "fourtitude/FourtitudeFormC.c",
        "fourtitude/FourtitudeLog.c",
        "fourtitude/FourtitudeStyle.c",
        "fourtitude/FourtitudeWindow.c",
    ],
    hdrs = glob(["fourtitude/*.h"]),
    strip_include_prefix = "/src",
    copts     = ["-std=c17"],
    linkopts  = select({
        "@bazel_tools//src/conditions:windows": ["-lgdi32", "-lwindowscodecs", "-lole32", "-lmsimg32"],
        "//conditions:default":                 ["-lX11"],
    }),
    linkstatic = True,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "fourtitude_cpp",
    srcs = [
        "fourtitude/FourtitudeDocument.cpp",
        "fourtitude/FourtitudeElement.cpp",
        "fourtitude/FourtitudeElements.cpp",
        "fourtitude/FourtitudeForm.cpp",
        "fourtitude/FourtitudeWindow.cpp",
    ],
    hdrs = ["fourtitude/fourtitudecpp.h"],
    strip_include_prefix = "/src",
    copts     = ["-std=c++17"],
    deps      = [":fourtitude_c"],
    linkstatic = True,
    visibility = ["//visibility:public"],
)