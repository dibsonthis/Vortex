#!/bin/sh


LIBS=""
RPATH="$ORIGIN/../lib"
if [ -z "$(ls -A modules/$1/lib/*)" ]; then
   LIBS=""
else
    LIBS="lib/*.so*"
fi

CONFIG=""
DIRECT_LIBS=""

if [ "$1" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE include/SDL2/*.cpp"
elif [ "$1" = "requests" ]; then
    DIRECT_LIBS=""
elif [ "$1" = "sqlite" ]; then
    CONFIG="-lsqlite3"
else
    CONFIG=$CONFIG
fi

cd "$PWD"/modules/"$1"

clang++ \
-dynamiclib \
-shared \
-g \
-fPIC \
-Wno-everything \
$CONFIG \
-Iinclude \
-Llib \
$LIBS \
-stdlib=libc++ \
-std=c++20 \
"$1".cpp  \
-o bin/"$1" \
$DIRECT_LIBS \
-Wl,--disable-new-dtags -Wl,-rpath,'$ORIGIN/../lib' 