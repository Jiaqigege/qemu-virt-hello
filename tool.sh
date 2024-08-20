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

help() {
    echo "Usage: $0 {build|b|run|r|clean|c|debug|d|help|h}"
    echo
    echo "Commands:"
    echo "  build (b)   Compile the kernel image (default action)"
    echo "  run (r)     Run the kernel using QEMU"
    echo "  clean (c)   Clean the build directory"
    echo "  debug (d)   Run the kernel in QEMU with debugging options (-s -S)"
    echo "  help (h)    Display this help message"
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
    help)
        help
        ;;
    *)
        echo "Invalid command: $action"
        help
        ;;
esac

