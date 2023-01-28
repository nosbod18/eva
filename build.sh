#!/bin/bash
cd "${0%/*}"
mkdir -p bin

# Build library
gcc -c src/eva.c -o bin/eva.o
ar rcs bin/libeva.a bin/eva.o
rm bin/eva.o

# Build demo
./demo/wtk/build.sh
if [[ "$(uname -s)" == "Darwin" ]]; then
    gcc demo/main.c -o bin/demo -Lbin -leva -Ldemo/wtk/bin -lwtk -framework Cocoa -framework OpenGL
else
    gcc demo/main.c -o bin/demo -Lbin -leva -Ldemo/wtk/bin -lwtk -lX11 -lGL
fi

