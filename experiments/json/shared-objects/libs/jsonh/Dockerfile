FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/sheredom/json.h json.h

WORKDIR /app/json.h
RUN git checkout 06aa5782d650e7b46c6444c2d0a048c0a1b3a072

WORKDIR /app
COPY libs/jsonh/Makefile.jsonh Makefile
COPY libs/jsonh/jsonh_adapter.c .

COPY generic_target.cpp .
COPY common.h .
