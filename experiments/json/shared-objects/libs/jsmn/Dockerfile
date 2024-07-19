FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/zserge/jsmn

WORKDIR /app/jsmn
RUN git checkout 25647e692c7906b96ffd2b05ca54c097948e879c

WORKDIR /app
COPY libs/jsmn/Makefile.jsmn Makefile
COPY libs/jsmn/jsmn_adapter.c .

COPY print-buffer.c .
COPY print-buffer.h .
COPY generic_target.cpp .
COPY common.h .