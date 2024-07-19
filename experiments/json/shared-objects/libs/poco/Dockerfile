FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/pocoproject/poco

WORKDIR /app/poco
RUN git checkout 69d15c5dddedf5d9adb86ade0dcee8254bb3ede5

WORKDIR /app
COPY libs/poco/Makefile.poco Makefile
COPY libs/poco/poco_adapter.cpp .

COPY generic_target.cpp .
COPY common.h .
