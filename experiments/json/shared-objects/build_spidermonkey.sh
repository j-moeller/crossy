# @Deprecated

#!/bin/sh
set -e

export MOZCONFIG=/app/experiments/json/shared-objects/MOZCONFIG_RELEASE
cd /app/spidermonkey
./mach build
