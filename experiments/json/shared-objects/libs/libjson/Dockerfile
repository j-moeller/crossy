FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/vincenthz/libjson

WORKDIR /app/libjson
RUN git checkout bd0cad8b3f063bedd95d27b32211b9fc388cf24f

WORKDIR /app
COPY libs/libjson/Makefile.libjson Makefile
COPY libs/libjson/libjson_adapter.c .

COPY print-buffer.c .
COPY print-buffer.h .
COPY generic_target.cpp .
COPY common.h .
