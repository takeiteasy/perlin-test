ifeq ($(OS),Windows_NT)
	PROG_EXT=.exe
	CFLAGS=-O2 -DSOKOL_D3D11 -lkernel32 -luser32 -lshell32 -ldxgi -ld3d11 -lole32 -lgdi32
	ARCH=win32
else
	UNAME:=$(shell uname -s)
	PROG_EXT=
	ifeq ($(UNAME),Darwin)
		CFLAGS=-x objective-c -DSOKOL_METAL -fno-objc-arc -framework Metal -framework Cocoa -framework MetalKit -framework Quartz -framework AudioToolbox -framework CoreFoundation -framework CoreServices -lpthread
		ARCH:=$(shell uname -m)
		ifeq ($(ARCH),arm64)
			ARCH=osx_arm64
		else
			ARCH=osx
		endif
	else ifeq ($(UNAME),Linux)
		CFLAGS=-DSOKOL_GLCORE33 -pthread -lGL -ldl -lm -lX11 -lasound -lXi -lXcursor -lpthread
		ARCH=linux
	else
		$(error OS not supported by this Makefile)
	endif
endif

CC=clang
SOURCE=$(wildcard src/*.c)
NAME=perlin

EXE=build/$(NAME)_$(ARCH)$(PROG_EXT)
JS=build/$(NAME).js
ARCH_PATH=bin/$(ARCH)

SHDC_PATH=$(ARCH_PATH)/sokol-shdc$(PROG_EXT)
SHADERS=$(wildcard assets/*.glsl)
SHADER_OUTS=$(patsubst %,%.h,$(SHADERS))

all: app

.SECONDEXPANSION:
SHADER=$(patsubst %.h,%,$@)
SHADER_OUT=$@
%.glsl.h: $(SHADERS)
	$(SHDC_PATH) -i $(SHADER) -o $(SHADER_OUT) -l glsl330:glsl100:glsl300es:hlsl4:metal_macos:wgpu
	mv $(SHADER_OUT) build/

shaders: $(SHADER_OUTS) cleanup

app: shaders
	$(CC) -Ibuild -Ideps -fenable-matrix $(CFLAGS) $(SOURCE) -o $(EXE)

web: shaders
	emcc -DSOKOL_GLES3 -Ibuild -Ideps -fenable-matrix $(SOURCE) -sUSE_WEBGL2=1 -o $(JS)

run: $(EXE)
	./$(EXE)

cleanup:
	rm assets/*.air
	rm assets/*.dia
	rm assets/*.metallib
	rm assets/*.metal

.PHONY: all app shaders run cleanup
