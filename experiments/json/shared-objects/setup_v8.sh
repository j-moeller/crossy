#!/bin/bash
set -e

#
# Building V8
#
# References:
# - https://v8.dev/docs/build
#
# Note that we already installed v8's dependencies in the Dockerfile.

# libs/v8/
mkdir -p libs/v8
cd libs/v8

gclient config https://github.com/j-moeller/v8
gclient sync

# libs/v8
cd v8
git checkout crossy

exit 0

# libs/v8/v8/paco
mkdir -p paco
cd paco

cd ..

# libs/v8/v8
gn gen out/paco_component --args=\
'is_asan=true '\
'is_debug=false '\
'sanitizer_coverage_flags="inline-8bit-counters,pc-table" '\
'sanitizer_coverage_allowlist_flags="v8_allowlist.txt" '\
'target_cpu="x64" '\
'is_component_build=true '\
'v8_use_external_startup_data=false'

# only copy if not already
cp ../../v8_allowlist.txt out/paco_component/v8_allowlist.txt

# allow to fail
git apply ../../../v8.patch

# TODO: find appropriate place to insert the strings
# v8/build/config/sanitizers/BUILD.gn -fsanitize-coverage-allowlist=$sanitizer_coverage_allowlist_flags
# v8/build/config/sanitizers/sanitizers.gni sanitizer_coverage_allowlist_flags = ""

ninja -C out/paco_component paco
