#!/bin/bash

CFLAGS="-std=c99 -Wall -Wextra -I.. -Iwtk"
LFLAGS="-Lbin -lwtk -leva"

if [[ "$(uname -s)" == "Darwin" ]]; then
    CFLAGS="$CFLAGS -x objective-c"
    LFLAGS="$LFLAGS -framework Cocoa -framework OpenGL"
else # Assume Linux
    LFLAGS="$LFLAGS -lX11 -lGL"
fi

mkdir -p bin

# Build wtk
gcc -c wtk/wtk/wtk.c
ar crs bin/libwtk.a wtk.o
rm wtk.o

# Build eva
gcc -c ../src/eva.c
ar crs bin/libeva.a eva.o
rm eva.o

# Build demos
gcc $CFLAGS triangle.c -o bin/triangle $LFLAGS
gcc $CFLAGS square.c -o bin/square $LFLAGS