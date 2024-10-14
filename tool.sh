#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Default QEMU path (this will be used to form the QEMU command, e.g., /usr/bin/qemu-system-aarch64)
QEMU_PATH=""

# Function to convert a path to an absolute path
get_abs_path() {
    echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

# Check if .iaqenv file exists and source it, otherwise search for QEMU binary in PATH
if [ -f ".iaqenv" ]; then
    source .iaqenv
fi

build() {
    rm -rf build && mkdir build

    echo "Compiling kernel image..."
    cd build

    cmake -G "Unix Makefiles" ..
    make

    exit 0
}

run() {
    ${QEMU_PATH}qemu-system-aarch64 -machine virt -cpu cortex-a57 -smp 1 -m 2G -kernel build/kernel.img -nographic
}

clean() {
    echo "Cleaning build directory..."
    rm -rf build
    exit 0
}

debug() {
    ${QEMU_PATH}qemu-system-aarch64 -machine virt -cpu cortex-a57 -smp 1 -m 2G -kernel build/kernel.img -nographic -s -S
}

export_dts() {
    echo "Exporting DTS from DTB..."
    ${QEMU_PATH}qemu-system-aarch64 -machine virt,dumpdtb=virt.dtb -cpu cortex-a57 -smp 1 -m 2G -nographic
    dtc -I dtb -O dts -o virt.dts virt.dtb
    rm virt.dtb
    echo "DTS file generated: virt.dts"
    exit 0
}

set_qemu_path() {
    if [ -z "$2" ]; then
        echo "Error: No path specified for QEMU."
        exit 1
    fi

    # Convert the provided path to an absolute path
    ABS_PATH=$(get_abs_path "$2")

    # Check if the specified QEMU path contains the qemu-system-aarch64 binary
    if ! [ -x "$ABS_PATH/qemu-system-aarch64" ]; then
        echo "Error: qemu-system-aarch64 not found in the specified path."
        exit 1
    fi

    # Write the absolute QEMU path to .iaqenv
    echo "QEMU_PATH=\"$ABS_PATH/\"" > .iaqenv
    echo "QEMU path set to '$ABS_PATH/' and saved to .iaqenv"
    exit 0
}

help() {
    echo "Usage: $0 {build|b|run|r|clean|c|debug|d|export_dts|e|set_qemu_path|p|help|h}"
    echo
    echo "Commands:"
    echo "  build (b)           Compile the kernel image (default action)"
    echo "  run (r)             Run the kernel using QEMU"
    echo "  clean (c)           Clean the build directory"
    echo "  debug (d)           Run the kernel in QEMU with debugging options (-s -S)"
    echo "  export_dts (e)      Export DTS from the generated DTB"
    echo "  set_qemu_path (p)   Set the path to QEMU executable and save to .iaqenv"
    echo "  help (h)            Display this help message"
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
        p) action="set_qemu_path" ;;
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
    set_qemu_path)
        set_qemu_path "$@"
        ;;
    help)
        help
        ;;
    *)
        echo "Invalid command: $action"
        help
        ;;
esac
