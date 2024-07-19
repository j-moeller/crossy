FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/open-source-parsers/jsoncpp

WORKDIR /app/jsoncpp
RUN git checkout 8190e061bc2d95da37479a638aa2c9e483e58ec6

WORKDIR /app
COPY libs/jsoncpp/Makefile.jsoncpp Makefile
COPY libs/jsoncpp/jsoncpp_adapter.cpp .

COPY print-buffer.c .
COPY print-buffer.h .
COPY generic_target.cpp .
COPY common.h .