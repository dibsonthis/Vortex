#!/bin/sh


LIBS=""
RPATH="@loader_path/../lib"
if [ -z "$(ls -A modules/$1/lib/*)" ]; then
   LIBS=""
else
    LIBS="lib/*.dll"
fi

echo $LIBS

CONFIG=""
DIRECT_LIBS=""

if [ "$1" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE include/SDL2/*.cpp lib/*.dll"
    DIRECT_LIBS="-lsdl2 -lsdl2_image -lsdl2_ttf -lsdl2_mixer"
elif [ "$1" = "requests" ]; then
    CONFIG="lib/*.dll"
    DIRECT_LIBS="-Llib -lws2_32 -l:libssl-3-x64.dll -l:libcrypto-3-x64.dll -lcrypt32"
elif [ "$1" = "sqlite" ]; then
    DIRECT_LIBS="-lsqlite3"
elif [ "$1" = "websockets" ]; then
    # CONFIG="lib/*.dll Vortex/**/*.cpp"
    CONFIG="lib/*.dll ../../../src/**/*.cpp"
    DIRECT_LIBS="-Llib -l:libssl-3-x64.dll -l:libcrypto-3-x64.dll -lcrypt32 -DWIN32_LEAN_AND_MEAN -lpthread -lws2_32 -lmswsock"
else
    CONFIG="-g"
fi

cd "$PWD"/modules/"$1"

clang++ \
-std=c++20 \
-dynamiclib \
-shared \
-Wno-everything \
$CONFIG \
$LIBS \
-Iinclude \
-Llib \
"$1".cpp  \
-o bin/"$1" \
$DIRECT_LIBS

cp lib/*.dll .