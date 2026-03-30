load("@rules_cc//cc:defs.bzl", "cc_toolchain")
load("@rules_cc//cc/private/toolchain:unix_cc_toolchain_config.bzl", "cc_toolchain_config")

cc_toolchain_config(
    name                    = "gcc_config",
    cpu                     = "x86_64",
    compiler                = "gcc",
    toolchain_identifier    = "gcc-ucrt64-windows",
    host_system_name        = "local",
    target_system_name      = "local",
    target_libc             = "ucrt",
    abi_version             = "gcc",
    abi_libc_version        = "ucrt",
    cxx_builtin_include_directories = [
        "C:/msys64/ucrt64/include",
        "C:/msys64/ucrt64/x86_64-w64-mingw32/include",
        "C:/msys64/ucrt64/lib/gcc/x86_64-w64-mingw32/14.2.0/include",
        "C:/msys64/ucrt64/lib/gcc/x86_64-w64-mingw32/14.2.0/include-fixed",
    ],
    tool_paths = {
        "gcc":     "C:/msys64/ucrt64/bin/gcc.exe",
        "g++":     "C:/msys64/ucrt64/bin/g++.exe",
        "ar":      "C:/msys64/ucrt64/bin/ar.exe",
        "ld":      "C:/msys64/ucrt64/bin/ld.exe",
        "nm":      "C:/msys64/ucrt64/bin/nm.exe",
        "objdump": "C:/msys64/ucrt64/bin/objdump.exe",
        "strip":   "C:/msys64/ucrt64/bin/strip.exe",
        "cpp":     "C:/msys64/ucrt64/bin/cpp.exe",
        "gcov":    "C:/msys64/ucrt64/bin/gcov.exe",
        "dwp":     "C:/msys64/ucrt64/bin/dwp.exe",
    },
    compile_flags           = [],
    cxx_flags               = [],
    link_flags              = ["-lgdi32"],
    link_libs               = ["-lstdc++", "-lsupc++"],
    unfiltered_compile_flags = [],
    dbg_compile_flags       = ["-g"],
    opt_compile_flags       = ["-O2"],
    opt_link_flags          = [],
    coverage_compile_flags  = [],
    coverage_link_flags     = [],
    supports_start_end_lib  = False,
)

filegroup(name = "empty")

cc_toolchain(
    name                  = "gcc_toolchain",
    toolchain_identifier  = "gcc-ucrt64-windows",
    toolchain_config      = ":gcc_config",
    all_files             = ":empty",
    compiler_files        = ":empty",
    dwp_files             = ":empty",
    linker_files          = ":empty",
    objcopy_files         = ":empty",
    strip_files           = ":empty",
    supports_param_files  = 0,
)

toolchain(
    name = "gcc_toolchain_def",
    exec_compatible_with = [
        "@platforms//os:windows",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:windows",
        "@platforms//cpu:x86_64",
    ],
    toolchain      = ":gcc_toolchain",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)
