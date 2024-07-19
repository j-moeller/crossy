FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/rustyrussell/ccan/

WORKDIR /app/ccan
RUN git checkout cd56b18ae87de6981b88abbe52544d8cc9f8aa0e

WORKDIR /app
COPY libs/ccan/Makefile.ccan Makefile
COPY libs/ccan/ccan_adapter.c .

COPY generic_target.cpp .
COPY common.h .