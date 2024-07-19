FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/j-moeller/frozen.git --depth 1

WORKDIR /app/frozen
RUN git checkout bba42efe684b66581aa335cf0ecd80c7b6a648ba

WORKDIR /app
COPY libs/frozen/Makefile.frozen Makefile
COPY libs/frozen/frozen_adapter.c .

COPY generic_target.cpp .
COPY common.h .
