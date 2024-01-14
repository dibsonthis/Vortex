#!/bin/sh

LIBS=""

cd modules
for FILE in *; 
do

mkdir $FILE/bin
mkdir $FILE/lib
mkdir $FILE/include

echo Compiling $FILE

if [ -z "$(ls -A $FILE/lib)" ]; then
   LIBS=""
else
    LIBS="$FILE/lib/lib*"
fi

CONFIG=""

if [ "$FILE" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL $FILE/include/SDL2/*.cpp"
elif [ "$FILE" = "requests" ]; then
    CONFIG="-framework CoreFoundation -framework Security"
elif [ "$FILE" = "sqlite" ]; then
    CONFIG="-lsqlite3"
elif [ "$FILE" = "websockets" ]; then
    CONFIG="-framework CoreFoundation -framework Security $FILE/Vortex/**/*.cpp"
    DIRECT_LIBS="-lssl -lcrypto"
else
    CONFIG=$CONFIG
fi

clang++ \
-dynamiclib \
-shared \
-Ofast \
-Wno-everything \
$CONFIG \
$LIBS \
-I$FILE/include \
-L$FILE/lib \
-std=c++20 \
-stdlib=libc++ \
$FILE/$FILE.cpp  \
-o $FILE/bin/$FILE \
-Wl,-rpath,@loader_path/../lib

done