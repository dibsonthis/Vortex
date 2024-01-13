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
    CONFIG="-lsqlite3"
elif [ "$1" = "websockets" ]; then
    CONFIG="lib/*.dll Vortex/**/*.cpp"
    DIRECT_LIBS="-Llib -lws2_32 -l:libssl-3-x64.dll -l:libcrypto-3-x64.dll -lcrypt32"
else
    CONFIG=$CONFIG
fi

cd "$PWD"/modules/"$1"

clang++ \
-dynamiclib \
-shared \
-g \
-Wno-everything \
$CONFIG \
$LIBS \
-Iinclude \
-Llib \
-std=c++20 \
"$1".cpp  \
-o bin/"$1" \
$DIRECT_LIBS \
# -Wl,-rpath,$RPATH

cp lib/*.dll .