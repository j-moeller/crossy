FROM debian:bookworm

RUN apt-get update
RUN apt-get install -y cmake make gcc clang-14 clang++-14 git coreutils
RUN apt-get install -y build-essential python3

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

RUN ln -s /usr/bin/clang++-14 /usr/bin/clang++
RUN ln -s /usr/bin/clang-14 /usr/bin/clang

RUN cp /usr/bin/tar /usr/bin/tar.orig
COPY libs/v8/tar-shim.sh /usr/bin/tar
RUN chmod u+x /usr/bin/tar

WORKDIR /app/v8-project
RUN gclient config --name v8 https://github.com/j-moeller/v8
RUN gclient sync

WORKDIR /app/v8-project/v8/paco
COPY libs/v8/v8_adapter.cpp .
COPY libs/v8/v8_allowlist.txt .

COPY generic_target.cpp .
COPY common.h .

WORKDIR /app
COPY libs/v8/Makefile.v8 Makefile

COPY libs/v8/prepare.py .
RUN python3 prepare.py