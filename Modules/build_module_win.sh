#!/bin/sh


LIBS=""
RPATH="@loader_path/../lib"
if [ -z "$(ls -A modules/$1/lib/lib*)" ]; then
   LIBS=""
else
    LIBS="lib/lib*"
fi

echo $LIBS

CONFIG=""

if [ "$1" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL include/SDL2/*.cpp"
elif [ "$1" = "requests" ]; then
    CONFIG="-Lws2_32"
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
"$1".cpp  \
-o bin/"$1" \
# -Wl,-rpath,$RPATH