build/spidermonkey/spidermonkey.so:
	docker build -t crossy-spidermonkey -f libs/spidermonkey/Dockerfile .
	docker run --rm -it \
		--env CC="${CC}" \
		--env CXX="${CXX}" \
		--env ENV_CFLAGS_NO_WERROR="${CFLAGS_NO_WERROR}" \
		--env ENV_CFLAGS="${CFLAGS}" \
		--env ENV_CFLAGS_SHARED_O="${CFLAGS_SHARED_O}" \
		--env ENV_CFLAGS_SHARED_LIB="${CFLAGS_SHARED_LIB}" \
		--env ENV_COV_ASAN_FLAGS="${COV_ASAN_FLAGS}" \
		--env ENV_COV_FULL_FLAGS="${COV_FULL_FLAGS}" \
		--env ENV_CFLAGS_LIBS="${CFLAGS_LIBS}" \
		--env ENV_CXX_FLAGS="${CXX_FLAGS}" \
		--env ENV_CXX_17_FLAGS="${CXX_17_FLAGS}" \
		--env ENV_LD_MAIN="${LD_MAIN}" \
		-v ${WORKDIR}/build/spidermonkey:/app/build \
		crossy-spidermonkey /bin/bash -c "make -j \$$(nproc --all)"