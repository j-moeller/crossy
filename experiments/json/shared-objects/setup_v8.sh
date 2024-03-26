#!/bin/bash
set -e

#
# Building V8
#
# References:
# - https://v8.dev/docs/build
#
# Note that we already installed v8's dependencies in the Dockerfile.

cd /build/v8

gclient config https://github.com/j-moeller/v8
gclient sync

cd v8
git checkout crossy

mkdir -p paco

gn gen out/paco_component --args=\
'is_asan=true '\
'is_debug=false '\
'sanitizer_coverage_flags="inline-8bit-counters,pc-table" '\
'sanitizer_coverage_allowlist_flags="v8_allowlist.txt" '\
'target_cpu="x64" '\
'is_component_build=true '\
'v8_use_external_startup_data=false'