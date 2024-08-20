#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

build() {
    rm -rf build && mkdir build

    echo "compiling kernel image..."
    cd build

    cmake -G "Unix Makefiles" ..
    make

    exit 0
}

run() {
    qemu-system-aarch64 -machine virt -cpu cortex-a57 -smp 1 -m 2G -kernel build/kernel.img -nographic
}

clean() {
    echo "Cleaning build directory..."
    rm -rf build
    exit 0
}

debug() {
    qemu-system-aarch64 -machine virt -cpu cortex-a57 -smp 1 -m 2G -kernel build/kernel.img -nographic -s -S
}

export_dts() {
    # The virt.dts file is generated using QEMU tools and is intended solely for inspecting the hardware resources. It is not actually used in the system's operation.
    echo "Exporting DTS from DTB..."
    qemu-system-aarch64 -machine virt,dumpdtb=virt.dtb -cpu cortex-a57 -smp 1 -m 2G -nographic
    dtc -I dtb -O dts -o virt.dts virt.dtb
    rm virt.dtb
    echo "DTS file generated: virt.dts"
    exit 0
}

help() {
    echo "Usage: $0 {build|b|run|r|clean|c|debug|d|export_dts|e|help|h}"
    echo
    echo "Commands:"
    echo "  build (b)        Compile the kernel image (default action)"
    echo "  run (r)          Run the kernel using QEMU"
    echo "  clean (c)        Clean the build directory"
    echo "  debug (d)        Run the kernel in QEMU with debugging options (-s -S)"
    echo "  export_dts (e)   Export DTS from the generated DTB"
    echo "  help (h)         Display this help message"
    exit 0
}

# Default action is to build
action="build"

# Check if an argument is passed
if [ $# -gt 0 ]; then
    case "$1" in
        b) action="build" ;;
        r) action="run" ;;
        c) action="clean" ;;
        d) action="debug" ;;
        e) action="export_dts" ;;
        h) action="help" ;;
        *) action="$1" ;;
    esac
fi

case "$action" in
    build)
        build
        ;;
    run)
        run
        ;;
    clean)
        clean
        ;;
    debug)
        debug
        ;;
    export_dts)
        export_dts
        ;;
    help)
        help
        ;;
    *)
        echo "Invalid command: $action"
        help
        ;;
esac

