FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

ENV IN_DOCKER_CONTAINER=1

RUN apt update
RUN apt install -y python3.10 python3.10-venv python3-pip clang-14 libc++-14-dev \
libc++abi-14-dev build-essential git vim software-properties-common npm curl \
ninja-build cmake openjdk-17-jdk openjdk-17-jre maven gdb

# create symbolic link for python2 because the V8 build tools require it
RUN ln -s /usr/bin/python2 /usr/bin/python

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

## set JAVA_HOME
ENV JAVA_HOME /usr/lib/jvm/java-17-openjdk-amd64/
RUN export JAVA_HOME

## install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain stable -y

# This is required for our rust libraries
RUN $HOME/.cargo/bin/rustup install nightly
RUN $HOME/.cargo/bin/rustup default nightly

RUN pip install -U --no-cache-dir orjson simplejson numpy posix-ipc

COPY ./entrypoint.sh /entrypoint.sh
RUN chmod u+x /entrypoint.sh

WORKDIR /app

# We need to initialize some things in _every_ container. The entrypoint does
# this for us and then executes our script.
ENTRYPOINT ["/entrypoint.sh"]
