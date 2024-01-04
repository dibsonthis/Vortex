#!/bin/sh

LIBS=""

rm -r bin/build/modules/win
mkdir -p bin/build/modules/win
pushd Modules/modules
for FILE in *; 
do

rm -r $FILE/bin
mkdir $FILE/bin
mkdir $FILE/lib
mkdir $FILE/include

echo Compiling $FILE

if [ -z "$(ls -A $FILE/lib)" ]; then
   LIBS=""
else
    LIBS="$FILE/lib/*.dll"
fi

CONFIG=""
DIRECT_LIBS=""

if [ "$FILE" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE $FILE/include/SDL2/*.cpp $FILE/lib/*.dll"
    DIRECT_LIBS="-lsdl2 -lsdl2_image -lsdl2_ttf -lsdl2_mixer"
elif [ "$FILE" = "requests" ]; then
    CONFIG="$FILE/lib/*.dll"
    DIRECT_LIBS="-L$FILE/lib -lws2_32 -l:libssl-3-x64.dll -l:libcrypto-3-x64.dll -lcrypt32"
elif [ "$FILE" = "sqlite" ]; then
    CONFIG="-lsqlite3"
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
$DIRECT_LIBS \
-stdlib=libstdc++

cp $FILE/lib/*.dll $FILE

done

popd

cp -r Modules/modules/* bin/build/modules/win

pushd bin/build/modules/win

for _FILE in *;
do

rm -r $_FILE/include
rm "$_FILE/$_FILE.cpp"

done

popd