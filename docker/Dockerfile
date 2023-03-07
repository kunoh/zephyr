FROM ubuntu:20.04

ARG UID=1000
ARG GID=1000
ARG ZEPHYR_SDK_VERSION="0.14.2"


# Zephyr docker does not work with azure since it creates user with UID 1000
# Also some more vars are set to indicate ZEPHYR Base dirs, that we have it different place
ENV TZ=Europe/Copenhagen

RUN apt-get update && \
    apt install -y wget locales protobuf-compiler \
    software-properties-common

RUN add-apt-repository ppa:ubuntu-toolchain-r/ppa && \
    apt-get update

RUN wget https://apt.kitware.com/kitware-archive.sh && \
    bash kitware-archive.sh && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone

RUN DEBIAN_FRONTEND=noninteractive apt install --no-install-recommends -y git cmake ninja-build gperf \
    ccache dfu-util device-tree-compiler jq \
    python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
    build-essential make gcc-10 g++-10 cpp-10 gcc-10-multilib g++-10-multilib libsdl2-dev libmagic1 \
    clang-format-12 linux-libc-dev

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10

RUN mkdir -p /opt/toolchains && \
    cd /opt/toolchains && \
    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.gz && \
    wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/sha256.sum | shasum --check --ignore-missing && \
    tar xf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.gz && \
    cd zephyr-sdk-${ZEPHYR_SDK_VERSION} && \
    ls && \
    ./setup.sh -t all -c -h && \
    chmod -R a+rw /opt/toolchains

COPY requirements.txt .
RUN pip install -r /requirements.txt

# GCC expect the path to be /usr/include/asm but it is renamed to /usr/include/asm-generic in this Ubuntu image
RUN ln -s /usr/include/asm-generic /usr/include/asm

RUN /usr/sbin/locale-gen en_US.UTF-8

ENV LANG=en_US.UTF-8
ENV ZEPHYR_REQUIREMENTS=v3.1.0
env ZEPHYR_TOOLCHAIN_VARIANT=zephyr
env ZEPHYR_SDK_INSTALL_DIR=/opt/toolchains/zephyr-sdk-${ZEPHYR_SDK_VERSION}

RUN /bin/sh -c "/bin/bash"
