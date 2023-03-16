# perlin-tool

Simple tool to play with Perlin noise in real time. **WIP**.

<p align="center">
    <img src="https://github.com/takeiteasy/perlin-tool/raw/master/assets/screenshotB.png"/>
    <img src="https://github.com/takeiteasy/perlin-tool/raw/master/assets/screenshotA.png"/>
</p>

## Build

To build, first run ```bootstrap.sh``` (this downloads dependency headers and the sokol shader build tools) then run ```make```. The binary will be inside the ```build/``` directory. Only tested on Mac but should build on Windows and Linux out of the box. If you're on Mac, you can also build the Xcode project with [Xcodegen](https://github.com/yonaskolb/XcodeGen).

**NOTE**: This program requires either clang or gcc to build. It won't build under MSVC or older compilers.

Alternatively, try the web version [here](https://takeiteasy.github.io/perlin-tool/). **NOTE**: The web version is missing some features.

## Roadmap for v1.0.0

- [ ] Load .obj files, display noise onto model
- [X] ~~Lua scripting to interact with heightmap~~
- [ ] User created shaders
- [X] ~~Hotreloading for assets~~
- [ ] Error handling for assets
- [ ] Touch controls
- [X] ~~Biome controls~~
- [X] ~~Config files + args~~

## Dependencies

- [floooh/sokol](https://github.com/floooh/sokol) (zlib/libpng)
    - sokol_gfx.h
    - sokol_app.h
    - sokol_glue.h
    - sokol_nuklear.h
    - sokol_args.h
- [Immediate-Mode-UI/Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) (MIT/Public Domain)
    - nuklear.h
- [miloyip/svpng](https://github.com/miloyip/svpng) (BSD-3-Clause)
    - svpng.inc
- [nothings/stb](https://github.com/nothings/stb/blob/master/deprecated/stretchy_buffer.h) (MIT/Public Domain)
    - stretchy_buffer.h (modified)
- [edubart/minilua](https://github.com/edubart/minilua) (MIT) ([Lua license](https://www.lua.org/license.html))
    - minilua.h
- [septag/dmon](https://github.com/septag/dmon) (BSD-2-Clause)
    - dmon.h
- [thisistherk/fast_obj](https://github.com/thisistherk/fast_obj) (MIT)
    - fast_obj.h
- [prideout/par](https://github.com/prideout/par) (MIT)
    - par_shapes.h
- [win32ports/dirent_h](https://github.com/win32ports/dirent_h/) (MIT)
    - dirent_win32.h
- [yohhoy/threads.h](https://gist.github.com/yohhoy/2223710) (Boost)
    - threads.h ([modified](https://gist.github.com/takeiteasy/f04ccebdaed5a9f554b99e7b4456198e))
- [tsoding/jim](https://github.com/tsoding/jim) (MIT)
    - jim.h
- [esr/microjson](https://gitlab.com/esr/microjson/) (BSD-2-clause)
    - mjson.h
- [AndrewBelt/osdialog](https://github.com/AndrewBelt/osdialog) (CC0)
    - osdialog.h ([modified](https://gist.github.com/takeiteasy/b8a89676eebcdc074362c0aec8ce5948))

## License
```
The MIT License (MIT)

Copyright (c) 2022 George Watson

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
