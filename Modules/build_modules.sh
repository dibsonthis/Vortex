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
    sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2-2.0.0.dylib" "$FILE/lib/libSDL2-2.0.0.dylib"
    CONFIG="-L/usr/local/share/vortex/modules/lib -I/usr/local/share/vortex/modules/SDL2 -D_THREAD_SAFE"
else
    CONFIG=""
fi

clang++ \
-dynamiclib \
-O3 \
-Wno-everything \
-I$FILE/include \
-L$FILE/lib \
$CONFIG \
$LIBS \
-std=c++20 \
-stdlib=libc++ \
"$PWD"/$FILE/$FILE.cpp \
-o "$PWD"/$FILE/bin/$FILE \
-Wl,-rpath "$PWD"/$FILE/lib

# if [ "$FILE" = "sdl" ]; then
#     sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2-2.0.0.dylib" "$FILE/lib/libSDL2-2.0.0.dylib"
#     sudo install_name_tool -change "$PWD"/$FILE/lib/libSDL2-2.0.0.dylib "/usr/local/share/vortex/modules/sdl/lib/libSDL2-2.0.0.dylib" $FILE/bin/sdl
# else
#     CONFIG=""
# fi

done