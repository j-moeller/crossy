build/v8/libv8-crossy.so:
	docker build -t crossy-v8 -f libs/v8/Dockerfile .
	docker run --rm -it \
		--env CC="${CC}" \
		--env CXX="${CXX}" \
		--env CFLAGS_NO_WERROR="${CFLAGS_NO_WERROR}" \
		--env CFLAGS="${CFLAGS}" \
		--env CFLAGS_SHARED_O="${CFLAGS_SHARED_O}" \
		--env CFLAGS_SHARED_LIB="${CFLAGS_SHARED_LIB}" \
		--env COV_ASAN_FLAGS="${COV_ASAN_FLAGS}" \
		--env COV_FULL_FLAGS="${COV_FULL_FLAGS}" \
		--env CFLAGS_LIBS="${CFLAGS_LIBS}" \
		--env CXX_FLAGS="${CXX_FLAGS}" \
		--env CXX_17_FLAGS="${CXX_17_FLAGS}" \
		--env LD_MAIN="${LD_MAIN}" \
		-v ${WORKDIR}/build/v8:/app/v8-project/v8/out/v8-crossy \
		crossy-v8 /bin/bash -c "make -j \$$(nproc --all)"