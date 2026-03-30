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
    hdrs = [
        "fourtitude/fourtitudec17.h",
        "fourtitude/FourtitudeStyle.h",
        "fourtitude/FourtitudeLog.h",
    ],
    includes  = ["."],
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
        "fourtitude/FourtitudeImage.cpp",
        "fourtitude/FourtitudeWindow.cpp",
    ],
    hdrs      = ["fourtitude/fourtitudecpp.h"],
    includes  = ["."],
    copts     = ["-std=c++17"],
    deps      = [":fourtitude_c"],
    linkstatic = True,
    visibility = ["//visibility:public"],
)