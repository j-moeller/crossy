FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/j-moeller/cJSON.git --depth 1

WORKDIR /app/cJSON
RUN git checkout 8601838022f06cd9ddbc27eba4202d6af96a2386

WORKDIR /app
COPY libs/cjson/Makefile.cjson Makefile
COPY libs/cjson/cjson_adapter.c .

COPY generic_target.cpp .
COPY common.h .
