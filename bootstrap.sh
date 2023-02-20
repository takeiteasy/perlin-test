#!/bin/sh

mkdir build

download() {
    wget "https://raw.githubusercontent.com/$1" -O "$2"deps/$(basename "$1")
}

# Download latest header-only dependencies
download "floooh/sokol/master/sokol_gfx.h"
download "floooh/sokol/master/sokol_glue.h"
download "floooh/sokol/master/sokol_app.h"
download "miloyip/svpng/master/svpng.inc"
download "Immediate-Mode-UI/Nuklear/master/nuklear.h"
download "floooh/sokol/master/util/sokol_nuklear.h"

cd tools

# Download sokol-shdc-tools binaries
rm -rf bin/ || true
git clone https://github.com/floooh/sokol-tools-bin
mkdir bin/
mv sokol-tools-bin/bin/* bin/
rm -rf sokol-tools-bin

cd ../
