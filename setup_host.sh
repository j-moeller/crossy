#!/bin/bash
set -e

echo "[Host] Initializing submodules"
git submodule update --init --recursive


echo "[Host] Fetching corpus"
git clone https://github.com/nlohmann/json_test_data /tmp/json_test_data || true
mkdir -p corpus/
find /tmp/json_test_data -size -5k -name "*.json" -exec cp "{}" "corpus/" \;


echo "[Host] Building docker container"
docker build -t mlsec-crossy .


echo "[Host] Starting initial container"
mkdir -p build
mkdir -p experiments/json/shared-objects/build
mkdir -p experiments/json/java/.m2/
docker run --rm -it \
    --env JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64" \
    -v $(pwd)/experiments:/app/experiments \
    -v $(pwd)/experiments/json/shared-objects/libs/spidermonkey:/app/experiments/json/shared-objects/libs/spidermonkey:ro \
    -v $(pwd)/experiments/json/shared-objects/libs/spidermonkey:/app/spidermonkey_readonly:ro \
    -v $(pwd)/ext/llvm-project:/app/ext/llvm-project:ro \
    -v $(pwd)/scripts:/app/scripts:ro \
    -v $(pwd)/src:/app/src:ro \
    -v $(pwd)/setup_container.sh:/app/setup_container.sh:ro \
    \
    -v $(pwd)/src/java:/app/src/java \
    -v $(pwd)/build:/app/build \
    mlsec-crossy /bin/bash -c "./setup_container.sh"
