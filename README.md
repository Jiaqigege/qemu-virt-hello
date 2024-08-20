# qemu-virt-hello

A helloworld kernel that runs on QEMU [virt](https://www.qemu.org/docs/master/system/arm/virt.html) machine.

# virt.dts

The virt.dts file is generated using QEMU tools and is intended solely for inspecting the hardware resources. It is not actually used in the system's operation. 

```shell
qemu-system-aarch64 -machine virt,dumpdtb=virt.dtb -cpu cortex-a57 -smp 1 -m 2G -nographic
dtc -I dtb -O dts -o virt.dts virt.dtb
```

## Credit

Some code is from ChCore, which is the lab of [MOSPI](https://ipads.se.sjtu.edu.cn/mospi/).
