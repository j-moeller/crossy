FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils

# Firefox dependencies
RUN apt-get install -y bash findutils gzip libxml2 m4 make perl tar unzip watchman
RUN apt-get install -y wget ccache pkg-config zlib1g-dev

RUN apt-get install -y curl

WORKDIR /app
RUN git clone https://github.com/j-moeller/gecko-dev/ --depth 1 gecko-dev

## install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain stable -y

# This is required for our rust libraries
RUN $HOME/.cargo/bin/rustup install 1.70
RUN $HOME/.cargo/bin/rustup default 1.70

COPY libs/spidermonkey/Makefile.spidermonkey Makefile
COPY libs/spidermonkey/spidermonkey_adapter.cpp .
COPY libs/spidermonkey/spidermonkey_allowlist.txt .
COPY libs/spidermonkey/MOZCONFIG_DEBUG .
COPY libs/spidermonkey/MOZCONFIG_RELEASE .

COPY generic_target.cpp .
COPY common.h .
