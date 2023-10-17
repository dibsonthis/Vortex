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
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2-2.0.0.dylib" "$FILE/lib/libSDL2-2.0.0.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2_image-2.0.0.dylib" "$FILE/lib/libSDL2_image-2.0.0.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2_ttf-2.0.0.dylib" "$FILE/lib/libSDL2_ttf-2.0.0.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2_mixer-2.0.0.dylib" "$FILE/lib/libSDL2_mixer-2.0.0.dylib"
    # CONFIG="-L/usr/local/share/vortex/modules/sdl/lib -I/usr/local/share/vortex/modules/sdl/include -D_THREAD_SAFE -framework GLUT -framework OpenGL"
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL"
elif [ "$FILE" = "requests" ]; then
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "$FILE/lib/libcrypto.3.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.dylib" "$FILE/lib/libcrypto.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.3.dylib" "$FILE/lib/libssl.3.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.dylib" "$FILE/lib/libssl.dylib"
    # sudo install_name_tool -change "/usr/local/lib/libcrypto.3.dylib" "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "$FILE/lib/libssl.dylib"
    # CONFIG="-framework CoreFoundation -framework Security -L/usr/local/share/vortex/modules/requests/lib -I/usr/local/share/vortex/modules/requests/include"
    CONFIG="-framework CoreFoundation -framework Security"
elif [ "$FILE" = "imgui" ]; then
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "modules/$1/lib/libcrypto.3.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.dylib" "modules/$1/lib/libcrypto.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.3.dylib" "modules/$1/lib/libssl.3.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.dylib" "modules/$1/lib/libssl.dylib"
    # sudo install_name_tool -change "/usr/local/lib/libcrypto.3.dylib" "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "modules/$1/lib/libssl.dylib"
    # CONFIG="-framework CoreFoundation -framework Security -L/usr/local/share/vortex/modules/requests/lib -I/usr/local/share/vortex/modules/requests/include"
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL $FILE/include/imgui/*.cpp"
elif [ "$FILE" = "sqlite" ]; then
    CONFIG="-lsqlite3"
else
    CONFIG=$CONFIG
fi

# clang++ \
# -dynamiclib \
# -Ofast \
# -Wno-everything \
# -I$FILE/include \
# -L$FILE/lib \
# $CONFIG \
# $LIBS \
# -std=c++20 \
# -stdlib=libc++ \
# "$PWD"/$FILE/$FILE.cpp \
# -o "$PWD"/$FILE/bin/$FILE \
# -Wl,-rpath "$PWD"/$FILE/lib

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