FROM ubuntu:20.04

ENV TZ=Europe/Copenhagen
RUN apt-get update && apt install -y wget sudo locales && wget https://apt.kitware.com/kitware-archive.sh && \
    sudo bash kitware-archive.sh && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    DEBIAN_FRONTEND=noninteractive sudo apt install --no-install-recommends -y git cmake ninja-build gperf \
    ccache dfu-util device-tree-compiler wget \
    python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
    make gcc gcc-multilib g++-multilib libsdl2-dev && \
    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.13.1/zephyr-sdk-0.13.1-linux-x86_64-setup.run && \
    chmod +x zephyr-sdk-0.13.1-linux-x86_64-setup.run && \
    ./zephyr-sdk-0.13.1-linux-x86_64-setup.run -- -d ~/zephyr-sdk-0.13.1 && \
    pip install west




RUN /usr/sbin/locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8

CMD ["/bin/sh" "-c" "/bin/bash"]
