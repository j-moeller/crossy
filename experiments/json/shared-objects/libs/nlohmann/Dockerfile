FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/nlohmann/json nlohmann

WORKDIR /app/nlohmann
RUN git checkout b2306145e1789368e6f261680e8dc007e91cc986

WORKDIR /app
COPY libs/nlohmann/Makefile.nlohmann Makefile
COPY libs/nlohmann/nlohmann_adapter.cpp .

COPY generic_target.cpp .
COPY common.h .