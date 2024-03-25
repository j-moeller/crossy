#!/bin/sh
set -e

export MOZCONFIG=/app/experiments/json/shared-objects/MOZCONFIG_RELEASE
cd /build/spidermonkey_readonly
./mach build

cd /build/spidermonkey
./mach build
