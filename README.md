# perlin-tool

Simple tool to play with Perlin noise in real time.

To build, first run ```bootstrap.sh``` (this downloads dependency headers and the sokol shader build tools) then run ```make```. The binary will be inside the ```build/``` directory. Only tested on Mac but should build on Windows (requires clang in PATH) and Linux out of the box.

Alternatively, try the web version [here](https://takeiteasy.github.io/perlin-tool/). **NOTE**: The web version is missing some features.

![Screenshot](/assets/screenshot.png)
<p align="center">
    <img src="https://github.com/takeiteasy/perlin-tool/raw/master/assets/recording.gif" />
</p>

## Roadmap for v1.0.0

- [ ] Load .obj files, display noise onto model
- [X] ~~Lua scripting to interact with heightmap~~
    - [ ] Improve error handling for Lua scripts
- [ ] User created shaders
- [ ] Hotreloading for assets

## Dependencies

- [floooh/sokol](https://github.com/floooh/sokol) (zlib/libpng)
    - sokol_gfx.h
    - sokol_app.h
    - sokol_glue.h
    - sokol_nuklear.h
- [Immediate-Mode-UI/Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) (MIT/Public Domain)
    - nuklear.h
- [miloyip/svpng](https://github.com/miloyip/svpng) (BSD-3-Clause)
    - svpng.inc
- [nothings/stb](https://github.com/nothings/stb/blob/master/deprecated/stretchy_buffer.h) (MIT/Public Domain)
    - stretchy_buffer.h (modified)
- [edubart/minilua](https://github.com/edubart/minilua) (MIT) ([Lua license](https://www.lua.org/license.html))
    - minilua.h

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
