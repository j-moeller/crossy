FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git curl coreutils

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

WORKDIR $HOME
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain stable -y

RUN $HOME/.cargo/bin/rustup install nightly
RUN $HOME/.cargo/bin/rustup default nightly

ENV PATH="$PATH:$HOME/.cargo/bin"

WORKDIR /app
COPY rust-deps/common common/
COPY libs/serde/serde_adapter/ serde_adapter/

COPY libs/serde/Makefile.serde Makefile

COPY rust-deps/rust_target.cpp .
COPY generic_target.cpp .
COPY common.h .