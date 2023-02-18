FROM ubuntu:20.04 AS builder

RUN sed -i '/deb-src/s/^# //' /etc/apt/sources.list && apt-get update
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get build-dep build-essential \
    gcc-defaults g++ gdb unzip pax bison flex texinfo python3-dev libpython2-dev \
    libncurses5-dev zlib1g-dev -y
RUN apt-get install python pax git unzip -y

ENV PATH="/opt/rtems/6/bin:$PATH"

WORKDIR /opt/rtems/src
RUN git clone git://git.rtems.org/rtems-source-builder.git rsb
RUN git clone git://git.rtems.org/rtems.git

FROM builder AS build_pc386
# i386 tool suite installation
WORKDIR /opt/rtems/src/rsb/rtems
RUN ../source-builder/sb-set-builder --prefix=/opt/rtems/6 6/rtems-i386

# pc386 BSP installation
WORKDIR /opt/rtems/src/rtems
RUN echo "[i386/pc386]" > config.ini && \
    echo "BUILD_TESTS = True" >> config.ini && \
    echo "RTEMS_POSIX_API = True" >> config.ini
RUN ./waf configure --prefix=/opt/rtems/6
RUN ./waf
RUN ./waf install
CMD bash

FROM builder AS build_raspi4b
# aarch64 tool suite installation
WORKDIR /opt/rtems/src/rsb/rtems
RUN ../source-builder/sb-set-builder --prefix=/opt/rtems/6 6/rtems-aarch64

# raspberrypi4b BSP installation
WORKDIR /opt/rtems/src/rtems
RUN echo "[aarch64/raspberrypi4b]" > config.ini && \
    echo "BUILD_TESTS = True" >> config.ini && \
    echo "RTEMS_POSIX_API = True" >> config.ini
RUN ./waf configure --prefix=/opt/rtems/6
RUN ./waf
RUN ./waf install
CMD bash

FROM ubuntu:20.04 AS pc386
ENV PATH="/opt/rtems/6/bin:$PATH"
RUN apt-get update && apt-get install python \
    qemu-system-i386 --no-install-recommends -y
COPY --from=build_pc386 /opt/rtems/6 /opt/rtems/6
COPY --from=build_pc386 /opt/rtems/src/rtems/build/i386/pc386/testsuites/samples /opt/rtems/src/samples
WORKDIR /app
CMD bash

FROM ubuntu:20.04 AS raspi4b
ENV PATH="/opt/rtems/6/bin:$PATH"
RUN apt-get update && apt-get install python --no-install-recommends -y
COPY --from=build_raspi4b /opt/rtems/6 /opt/rtems/6
COPY --from=build_raspi4b /opt/rtems/src/rtems/build/aarch64/raspberrypi4b/testsuites/samples /opt/rtems/src/samples
WORKDIR /app
CMD bash

FROM ubuntu:20.04 AS tmp
COPY --from=pc386 /opt/rtems/6 /tmp/pc386
COPY --from=raspi4b /opt/rtems/6 /tmp/raspi4b
RUN cp -rfn /tmp/pc386/* /tmp/raspi4b/

FROM ubuntu:20.04 AS merge
ENV PATH="/opt/rtems/6/bin:$PATH"
RUN apt-get update && apt-get install python \
    qemu-system-i386 --no-install-recommends -y
COPY --from=tmp /tmp/raspi4b /opt/rtems/6
COPY --from=pc386 /opt/rtems/src/samples /opt/rtems/src/i386/pc386/samples
COPY --from=raspi4b /opt/rtems/src/samples /opt/rtems/src/aarch64/raspberrypi4b/samples
WORKDIR /app
CMD bash
