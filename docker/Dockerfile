FROM ubuntu:20.04

ARG UID=1000
ARG GID=1000

# Zephyr docker does not work with azure since it creates user with UID 1000
# Also some more vars are set to indicate ZEPHYR Base dirs, that we have it different place
ENV TZ=Europe/Copenhagen
RUN apt-get update && \
    apt install -y wget locales protobuf-compiler && \
    wget https://apt.kitware.com/kitware-archive.sh && \
    bash kitware-archive.sh && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone && \
    DEBIAN_FRONTEND=noninteractive apt install --no-install-recommends -y git cmake ninja-build gperf \
    ccache dfu-util device-tree-compiler jq \
    python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
    make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 && \
    mkdir -p /opt/toolchains && \
    cd /opt/toolchains

RUN wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.14.2/zephyr-sdk-0.14.2_linux-x86_64.tar.gz && \
    wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.14.2/sha256.sum | shasum --check --ignore-missing && \
    tar xf zephyr-sdk-0.14.2_linux-x86_64.tar.gz && \
    cd zephyr-sdk-0.14.2 && \
    ls && \
    ./setup.sh -t all -c -h && \
    chmod -R a+rw /opt/toolchains

COPY requirements.txt .
RUN pip install -r /requirements.txt


RUN /usr/sbin/locale-gen en_US.UTF-8

ENV LANG=en_US.UTF-8
ENV ZEPHYR_REQUIREMENTS=v3.1.0
env ZEPHYR_TOOLCHAIN_VARIANT=zephyr
env ZEPHYR_SDK_INSTALL_DIR=/opt/toolchains/zephyr-sdk-0.14.2

RUN /bin/sh -c "/bin/bash"
