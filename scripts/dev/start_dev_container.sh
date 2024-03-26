#!/bin/bash
set -e

mkdir -p build/
mkdir -p output/
mkdir -p experiments/misc/build/

docker run -d -it \
    --env JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64" \
    -v $(pwd)/corpus:/app/corpus \
    -v $(pwd)/configs:/app/configs:ro \
    -v $(pwd)/experiments:/app/experiments:ro \
    -v $(pwd)/ext/llvm-project:/app/ext/llvm-project:ro \
    -v $(pwd)/scripts:/app/scripts:ro \
    -v $(pwd)/src:/app/src:ro \
    -v $(pwd)/Makefile:/app/Makefile:ro \
    -v $(pwd)/suppr.txt:/app/suppr.txt:ro \
    -v $(pwd)/setup_container.sh:/app/setup_container.sh:ro \
    \
    -v $(pwd)/experiments/json/java:/app/experiments/json/java \
    -v $(pwd)/experiments/json/java/.m2:/root/.m2 \
    -v $(pwd)/experiments/json/shared-objects/build:/app/experiments/json/shared-objects/build \
    -v $(pwd)/experiments/json/shared-objects/libs:/app/experiments/json/shared-objects/libs \
    -v $(pwd)/experiments/json/shared-objects/libs/spidermonkey:/build/spidermonkey \
    -v $(pwd)/experiments/json/shared-objects/libs/v8:/build/v8 \
    \
    -v $(pwd)/src/java:/app/src/java \
    -v $(pwd)/build:/app/build \
    -v $(pwd)/output:/app/output \
    -v $(pwd)/tests:/app/tests:ro \
    -v $(pwd)/experiments/json/shared-objects/rust:/app/experiments/json/shared-objects/rust \
    mlsec-crossy /bin/bash
