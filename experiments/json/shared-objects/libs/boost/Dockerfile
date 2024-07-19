FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/boostorg/boost boost

WORKDIR /app/boost
RUN git checkout e95d9c7ff9b7246254e64ac3f78e808af0c242b8
RUN git submodule update --init --recursive --depth 1

RUN ./bootstrap.sh --with-toolset=clang
RUN ./b2 headers

WORKDIR /app
COPY libs/boost/Makefile.boost Makefile
COPY libs/boost/boost_adapter.cpp .

COPY generic_target.cpp .
COPY common.h .