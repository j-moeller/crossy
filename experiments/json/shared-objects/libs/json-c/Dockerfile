FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/j-moeller/json-c

WORKDIR /app/json-c
RUN git checkout 9a77ba69e3a471b2b9bbdd4de5ec6c14b8c7102b

WORKDIR /app
COPY libs/json-c/Makefile.json-c Makefile
COPY libs/json-c/json-c_adapter.c .

COPY generic_target.cpp .
COPY common.h .
