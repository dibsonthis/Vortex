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
    CONFIG="-L/usr/local/share/vortex/modules/sdl/lib -I/usr/local/share/vortex/modules/sdl/include -D_THREAD_SAFE"
elif [ "$FILE" = "requests" ]; then
    sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "$FILE/lib/libcrypto.3.dylib"
    sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.dylib" "$FILE/lib/libcrypto.dylib"
    sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.3.dylib" "$FILE/lib/libssl.3.dylib"
    sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.dylib" "$FILE/lib/libssl.dylib"
    CONFIG="-framework CoreFoundation -framework Security -L/usr/local/share/vortex/modules/requests/lib -I/usr/local/share/vortex/modules/requests/include"
else
    CONFIG=$CONFIG
fi

clang++ \
-dynamiclib \
-Ofast \
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

done