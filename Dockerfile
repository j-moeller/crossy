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

## Compile node
# WORKDIR /deps
#RUN git clone https://github.com/nodejs/node

#WORKDIR /deps/node
#RUN git checkout v19.0.0
#RUN ./configure --ninja
#RUN make -j8

# Firefox dependencies
RUN apt-get install -y bash findutils gzip libxml2 m4 make perl tar unzip watchman

# V8 dependencies (from ./build/install-build-deps.sh)
RUN apt-get install -y binutils bison bzip2 cdbs curl dbus-x11 dpkg-dev \
    elfutils devscripts fakeroot flex git-core gperf libasound2-dev \
    libatspi2.0-dev libbrlapi-dev libbz2-dev libcairo2-dev libcap-dev \
    libc6-dev libcups2-dev libcurl4-gnutls-dev libdrm-dev libelf-dev \
    libevdev-dev libffi-dev libgbm-dev libglib2.0-dev libglu1-mesa-dev \
    libgtk-3-dev libkrb5-dev libnspr4-dev libnss3-dev libpam0g-dev libpci-dev \
    libpulse-dev libsctp-dev libspeechd-dev libsqlite3-dev libssl-dev \
    libsystemd-dev libudev-dev libva-dev libwww-perl libxshmfence-dev \
    libxslt1-dev libxss-dev libxt-dev libxtst-dev lighttpd locales openbox \
    p7zip patch perl pkg-config rpm ruby subversion uuid-dev wdiff x11-utils \
    xcompmgr xz-utils zip

RUN apt-get install -y lib32z1 libasound2 libatk1.0-0 libatspi2.0-0 libc6 \
    libcairo2 libcap2 libcgi-session-perl libcups2 libdrm2 libegl1 libevdev2 \
    libexpat1 libfontconfig1 libfreetype6 libgbm1 libglib2.0-0 libgl1 \
    libgtk-3-0 libpam0g libpango-1.0-0 libpangocairo-1.0-0 libpci3 libpcre3 \
    libpixman-1-0 libspeechd2 libstdc++6 libsqlite3-0 libuuid1 libwayland-egl1 \
    libwayland-egl1-mesa libx11-6 libx11-xcb1 libxau6 libxcb1 libxcomposite1 \
    libxcursor1 libxdamage1 libxdmcp6 libxext6 libxfixes3 libxi6 libxinerama1 \
    libxrandr2 libxrender1 libxtst6 x11-utils xvfb zlib1g

# install Google depot_tools
RUN git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git $HOME/depot_tools
ENV PATH="/root/depot_tools:$PATH"

## install rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain stable -y

# This is required for our rust libraries
RUN $HOME/.cargo/bin/rustup install nightly
RUN $HOME/.cargo/bin/rustup default nightly

# This is required for firefox
# TODO: Switch back to nightly after firefox has finished compiling
RUN $HOME/.cargo/bin/rustup install 1.70.0
RUN $HOME/.cargo/bin/rustup default 1.70.0
RUN echo 'source $HOME/.cargo/env' >> $HOME/.bashrc

RUN pip install -U --no-cache-dir orjson simplejson numpy posix-ipc

COPY ./entrypoint.sh /entrypoint.sh
RUN chmod u+x /entrypoint.sh

RUN cp /usr/bin/tar /usr/bin/tar.orig
COPY tar-shim.sh /usr/bin/tar
RUN chmod u+x /usr/bin/tar

WORKDIR /app

# We need to initialize some things in _every_ container. The entrypoint does
# this for us and then executes our script.
ENTRYPOINT ["/entrypoint.sh"]
