FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/akheron/jansson

WORKDIR /app/jansson
RUN git checkout a22dc95311a79f07b68fdfeefe3b06eb793d3bc9

WORKDIR /app
COPY libs/jansson/Makefile.jansson Makefile
COPY libs/jansson/jansson_adapter.c .

COPY generic_target.cpp .
COPY common.h .
