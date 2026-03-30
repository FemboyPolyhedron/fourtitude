bazel build //src:fourtitude_c;
bazel build //src:fourtitude_cpp;
Remove-Item -Path ./obj/* -Force;
Remove-Item -Path ./src/prebuilt/* -Force;
Copy-Item  -Path ./bazel-bin/src/libfourtitude_c.a -Destination ./obj/;
Copy-Item  -Path ./bazel-bin/src/libfourtitude_cpp.a -Destination ./obj/;
Copy-Item  -Path ./bazel-bin/src/libfourtitude_c.a -Destination ./src/prebuilt/;
Copy-Item  -Path ./bazel-bin/src/libfourtitude_cpp.a -Destination ./src/prebuilt/;