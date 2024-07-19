FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR /app
RUN git clone https://github.com/json-parser/json-builder

WORKDIR /app/json-builder
RUN git checkout fffd849bd03ce6c00ec2d9dd7a7cc0a255700c68

WORKDIR /app
RUN git clone https://github.com/json-parser/json-parser

WORKDIR /app/json-parser
RUN git checkout 94f66d8f83c1d84ccccd7540ec2f51cf8325d272

WORKDIR /app
COPY libs/json-parser/Makefile.json-parser Makefile
COPY libs/json-parser/json-parser_adapter.c .

COPY generic_target.cpp .
COPY common.h .