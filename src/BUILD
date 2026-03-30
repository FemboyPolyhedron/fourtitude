load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

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

# ---------------------------------------------------------------------------
# Source-build targets  (use from D:/fourtitude workspace only)
# ---------------------------------------------------------------------------

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
    copts    = ["-std=c17"],
    linkopts = select({
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

# ---------------------------------------------------------------------------
# Prebuilt-import targets  (used by consumers such as kineriggy)
# Requires prebuilt/*.a to exist — run build.ps1 from D:/fourtitude first.
# ---------------------------------------------------------------------------

cc_import(
    name = "_fourtitude_c_import",
    static_library = "prebuilt/libfourtitude_c.a",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "fourtitude_c_prebuilt",
    hdrs = glob(["fourtitude/*.h"]),
    strip_include_prefix = "/src",
    linkopts = select({
        "@bazel_tools//src/conditions:windows": ["-lgdi32", "-lwindowscodecs", "-lole32", "-lmsimg32"],
        "//conditions:default":                 ["-lX11"],
    }),
    deps = [":_fourtitude_c_import"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "_fourtitude_cpp_import",
    static_library = "prebuilt/libfourtitude_cpp.a",
    visibility = ["//visibility:private"],
)

cc_library(
    name = "fourtitude_cpp_prebuilt",
    hdrs = ["fourtitude/fourtitudecpp.h"],
    strip_include_prefix = "/src",
    deps = [":_fourtitude_cpp_import", ":fourtitude_c_prebuilt"],
    visibility = ["//visibility:public"],
)
