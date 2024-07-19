FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/lloyd/yajl

WORKDIR /app/yajl
RUN git checkout 5e3a7856e643b4d6410ddc3f84bc2f38174f2872

WORKDIR /app
COPY libs/yajl/Makefile.yajl Makefile
COPY libs/yajl/yajl_adapter.c .

COPY generic_target.cpp .
COPY common.h .
