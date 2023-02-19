# Sample task sets implementation in RTEMS
This project implements sample task sets defined by Preemptive Time Petri Nets (PTPNs) with the RTEMS real-time operating system. Timestamps of events are gathered to be then verified with the [ORIS Tool](https://stlab.dinfo.unifi.it/oris1.0/).

## Running
Compiled executables can be found in the [Releases](https://github.com/marcodiri/rtems-task-sets/releases) page.
### Simulator
Executables compiled for ```aarch64/xilinx_zynqmp_lp64_qemu``` can be run on the QEMU emulator with the following command:
```shell
> qemu-system-aarch64 -no-reboot -nographic -serial mon:stdio \
-machine xlnx-zcu102 -m 4096 -kernel example.exe
 ```
### Raspberry Pi 4 Model B
Along executables compiled for ```aarch64/raspberrypi4b``` you will also find the corresponding kernel image.

- Copy `kernel8.img` to the SD card boot partition.
- Console I/O is available through UART, which you can enable following the [UART Setup](https://docs.rtems.org/branches/master/user/bsps/bsps-aarch64.html#uart-setup) section in the docs.

## Building from source
RTEMS 6 and Board Support Packages (BSPs) are needed in order to build applications.

The [Dockerfile](https://github.com/marcodiri/rtems-task-sets/blob/master/Dockerfile) is available for easy reproducibility of the environment:
```shell
> git clone --recurse-submodules https://github.com/marcodiri/rtems-task-sets.git
> cd rtems-task-sets
> DOCKER_BUILDKIT=1 docker build -t rtems:6 .
 ```
 Then compile the applications in a container (pass the absolute path to the `./app` folder before the colon):
 ```shell
> docker run --rm -it -v /absolute/path/to/app/folder:/app rtems:6 
> chmod +x ./waf

# build for raspberry
> ./waf configure --rtems=/opt/rtems/6 --rtems-bsp=aarch64/raspberrypi4b
> ./waf

# build for xilinx_zynqmp_lp64_qemu
> ./waf configure --rtems=/opt/rtems/6 --rtems-bsp=aarch64/xilinx_zynqmp_lp64_qemu
> ./waf
 ```
 Outputs will be in the `build` directory.