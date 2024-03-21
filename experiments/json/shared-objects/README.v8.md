# Building the V8 JSON adapter
We must use the V8 build system to compile and link the v8.so shared library that is imported by the fuzzer.
Only by using gn and ninja we can successfully link the executables.

We currently are trying to automate this process (see: build_v8.sh), but until then we need to setup v8 manually. Please follow the steps from https://v8.dev/docs/build.

Our implementation is based on:

```
git checkout 23eefbee538ddd13363cddf864be45cf68fc7404
```

## Preparation
To build our own executables we must apply the following changed to [BUILD.gn](./v8/BUILD.gn):

- Add `cflags = [ "-fsanitize-recover=address"]` to the `config("internal_config_base")` block to set the flag for all executables.
- Set two new targets for the shared lib and the standalone. They are renamed later, because naming the library `v8.so` breaks the build system.
```
v8_shared_library("paco") {
  sources = [
    "paco/common.h",
    "paco/generic_target.cpp",
    "paco/v8_adapter.cpp",
  ]

  configs = [
    # Note: don't use :internal_config here because this target will get
    # the :external_config applied to it by virtue of depending on :v8, and
    # you can't have both applied to the same target.
    ":internal_config_base",
  ]

  deps = [
    ":v8",
    ":v8_libbase",
    ":v8_libplatform",
    "//build/win:default_exe_manifest",
  ]
}
```

We create a folder `paco` that contains symlinks to all our source files.

## Enabling instrumentation

- Copy `v8_allowlist.txt` to `v8/out/paco_component/`
- Modify `config("coverage_flags")` in `v8/build/config/sanitizers/BUILD.gn` to include:
```
"-fsanitize-coverage-allowlist=$sanitizer_coverage_allowlist_flags",
```
in its cflags below `-fsanitize-coverage`.
- Modify `declare_args()` in `v8/build/config/sanitizers/sanitizers.gni` to include:
```
sanitizer_coverage_allowlist_flags = ""
```
after the definition of `sanitizer_coverage_flags`.

## Generating ninja build files

The config can be generated using `gn`:

```
gn gen out/paco_component --args='is_asan=true is_debug=false sanitizer_coverage_flags="inline-8bit-counters,pc-table" sanitizer_coverage_allowlist_flags="v8_allowlist.txt" target_cpu="x64" is_component_build=true v8_use_external_startup_data=false'
```

We set these args to receive the same compiler flags as for our `COVERAGE_FLAGS`.

## Build

The build can be executed using `ninja -C out/paco_component`.
