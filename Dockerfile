FROM ubuntu:20.04 AS builder

RUN sed -i '/deb-src/s/^# //' /etc/apt/sources.list && apt-get update
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get build-dep build-essential \
    gcc-defaults g++ gdb unzip pax bison flex texinfo python3-dev libpython2-dev \
    libncurses5-dev zlib1g-dev -y
RUN apt-get install python pax git unzip \
    qemu-system-aarch64 --no-install-recommends -y

ENV PATH="/opt/rtems/6/bin:$PATH"

WORKDIR /opt/rtems/src
RUN git clone git://git.rtems.org/rtems-source-builder.git rsb
RUN git clone git://git.rtems.org/rtems.git

# aarch64 tool suite installation
WORKDIR /opt/rtems/src/rsb/rtems
RUN ../source-builder/sb-set-builder --prefix=/opt/rtems/6 6/rtems-aarch64

FROM builder AS bsps
# xilinx_zynqmp_lp64_qemu BSP
WORKDIR /opt/rtems/src/rtems
RUN echo "[aarch64/xilinx_zynqmp_lp64_qemu]" > config.ini && \
    echo "RTEMS_POSIX_API = True" >> config.ini
# raspberrypi4b BSP
RUN echo "[aarch64/raspberrypi4b]" >> config.ini && \
    echo "BUILD_TESTS = True" >> config.ini && \
    echo "RTEMS_POSIX_API = True" >> config.ini
RUN ./waf configure --prefix=/opt/rtems/6
RUN ./waf
RUN ./waf install

WORKDIR /app
CMD bash
