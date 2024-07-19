WORKDIR := $(shell pwd)
UUID := $(shell date +"%Y-%m-%dT%H.%M.%S%z")

all: build/crossy shared_libs java_libs

.prepare:
	git submodule update --init --recursive --depth 1

build/crossy: .prepare
	docker build -t crossy-main -f Dockerfile.crossy .
	docker run --rm -it \
		-v ${WORKDIR}/build/:/app/build \
		crossy-main /bin/bash -c "make -j \$$(nproc --all)"


# Shared libraries
shared_libs:
	cd experiments/json/shared-objects && make

# Java
java_libs:
	cd experiments/json/java/java-adapter && make

run:
	mkdir -p output/${UUID}

	docker build -t crossy-run -f Dockerfile .
	docker run --rm -it \
		-v ${WORKDIR}/build/:/app/build:ro \
		-v ${WORKDIR}/corpus-final/:/app/corpus:ro \
		-v ${WORKDIR}/configs/:/app/configs:ro \
		-v ${WORKDIR}/experiments/:/app/experiments:ro \
		-v ${WORKDIR}/output/${UUID}/:/app/output \
		crossy-run