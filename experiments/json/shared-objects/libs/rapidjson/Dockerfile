FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/Tencent/rapidjson

WORKDIR /app/rapidjson
RUN git checkout 012be8528783cdbf4b7a9e64f78bd8f056b97e24

WORKDIR /app
COPY libs/rapidjson/Makefile.rapidjson Makefile
COPY libs/rapidjson/rapidjson_adapter.cpp .

COPY generic_target.cpp .
COPY common.h .