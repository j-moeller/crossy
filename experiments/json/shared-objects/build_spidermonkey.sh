#!/bin/sh
set -e

export MOZCONFIG=/app/experiments/json/shared-objects/MOZCONFIG_RELEASE

rustup default 1.70.0
cd /build/spidermonkey
./mach build

rustup default nightly
