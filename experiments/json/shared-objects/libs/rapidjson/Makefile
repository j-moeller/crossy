build/rapidjson/rapidjson.so:
	docker build -t crossy-rapidjson -f libs/rapidjson/Dockerfile .
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
		-v ${WORKDIR}/build/rapidjson:/app/build \
		crossy-rapidjson /bin/bash -c "make -j \$$(nproc --all)"