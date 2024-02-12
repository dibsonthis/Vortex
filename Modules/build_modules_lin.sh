#!/bin/sh

LIBS=""

cd modules
for FILE in *; 
do

rm -rf $FILE/bin
mkdir $FILE/bin
mkdir $FILE/lib
mkdir $FILE/include

echo Compiling $FILE

if [ -z "$(ls -A $FILE/lib)" ]; then
   LIBS=""
else
    LIBS="$FILE/lib/*.so*"
fi

CONFIG=""
DIRECT_LIBS=""

if [ "$FILE" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE $FILE/include/SDL2/*.cpp"
elif [ "$FILE" = "requests" ]; then
    DIRECT_LIBS=""
elif [ "$FILE" = "sqlite" ]; then
    CONFIG="-lsqlite3"
elif [ "$FILE" = "websockets" ]; then
    # CONFIG="$FILE/Vortex/**/*.cpp"
    CONFIG="../../src/**/*.cpp"
    DIRECT_LIBS=""
else
    CONFIG=$CONFIG
fi

clang++ \
-dynamiclib \
-shared \
-Ofast \
-fPIC \
-Wno-everything \
$CONFIG \
$LIBS \
-I$FILE/include \
-L$FILE/lib \
-std=c++20 \
-stdlib=libc++ \
$FILE/$FILE.cpp  \
-o $FILE/bin/$FILE \
$DIRECT_LIBS \
-Wl,--disable-new-dtags -Wl,-rpath,'$ORIGIN/../lib'

done