#!/bin/sh

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
download "septag/dmon/master/dmon.h"
download "edubart/minilua/main/minilua.h"
download "thisistherk/fast_obj/master/fast_obj.h"
download "prideout/par/master/par_shapes.h"
download "win32ports/dirent_h/master/dirent.h"
mv deps/dirent.h deps/dirent_win32.h
wget "https://gist.githubusercontent.com/takeiteasy/f04ccebdaed5a9f554b99e7b4456198e/raw/2b09989d9ac50f849d5fb21ea1ad4cc33e0b363d/threads.h" -O deps/threads.h

# Download sokol-shdc-tools binaries
rm -rf bin/ || true
git clone https://github.com/floooh/sokol-tools-bin
mkdir bin/
mv sokol-tools-bin/bin/* bin/
rm -rf sokol-tools-bin
