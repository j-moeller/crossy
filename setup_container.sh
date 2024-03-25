#!/bin/bash
set -e

echo "[Container]: Setting up container"

if [ -z ${IN_DOCKER_CONTAINER} ]; then
    echo "Error: Not in docker container"
    exit 1
fi

echo "[Container]: Setting up dependencies"
cd experiments/json/java && ./setup.sh && cd -
# cd experiments/json/shared-objects && ./build_spidermonkey.sh
# cd experiments/json/shared-objects && ./setup_v8.sh

echo "[Container]: Build project"
make build