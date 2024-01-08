#!/bin/sh


LIBS=""
RPATH="@loader_path/../lib"
if [ -z "$(ls -A modules/$1/lib)" ]; then
   LIBS=""
else
    LIBS="lib/lib*"
fi

CONFIG=""
DIRECT_LIBS=""

if [ "$1" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL include/SDL2/*.cpp"
elif [ "$1" = "requests" ]; then
    CONFIG="-framework CoreFoundation -framework Security"
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
-Wno-everything \
$CONFIG \
$LIBS \
-Iinclude \
-Llib \
-std=c++20 \
-stdlib=libc++ \
"$1".cpp  \
-o bin/"$1" \
-arch arm64 \
-arch x86_64 \
-Wl,-rpath,$RPATH