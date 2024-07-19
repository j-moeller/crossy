WORKDIR := $(shell pwd)

all: build/crossy shared_libs java_libs

.prepare:
	git submodule update --init --recursive

build/crossy: .prepare
	docker build -t crossy-main -f Dockerfile.crossy .
	docker run --rm -it \
		-v ${WORKDIR}/build/:/app/build \
		crossy-main /bin/bash -c "make"


# Shared libraries
shared_libs:
	cd experiments/json/shared-objects && make

# Java
java_libs:
	cd experiments/json/java/java-adapter && make

run:
	docker build -t crossy-run -f Dockerfile .