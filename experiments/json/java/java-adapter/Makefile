WORKDIR := $(shell pwd)

target/java-adapter-1.0.jar:
	docker build -t crossy-java-adapter -f Dockerfile .
	docker run --rm -it \
		-v ${WORKDIR}/target/:/app/target \
		-v ${WORKDIR}/src/:/app/src:ro \
		crossy-java-adapter /bin/bash -c "make -j \$$(nproc --all)"
