#!/bin/sh


LIBS=""
RPATH="@loader_path/../lib"
if [ -z "$(ls -A modules/$1/lib)" ]; then
   LIBS=""
else
    LIBS="lib/lib*"
fi

CONFIG=""

if [ "$1" = "sdl" ]; then
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2-2.0.0.dylib" "modules/$1/lib/libSDL2-2.0.0.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2_image-2.0.0.dylib" "modules/$1/lib/libSDL2_image-2.0.0.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2_ttf-2.0.0.dylib" "modules/$1/lib/libSDL2_ttf-2.0.0.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/sdl/lib/libSDL2_mixer-2.0.0.dylib" "modules/$1/lib/libSDL2_mixer-2.0.0.dylib"
    # CONFIG="-L/usr/local/share/vortex/modules/sdl/lib -I/usr/local/share/vortex/modules/sdl/include -D_THREAD_SAFE -framework GLUT -framework OpenGL"
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL include/SDL2/*.cpp"
elif [ "$1" = "requests" ]; then
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "modules/$1/lib/libcrypto.3.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libcrypto.dylib" "modules/$1/lib/libcrypto.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.3.dylib" "modules/$1/lib/libssl.3.dylib"
    # sudo install_name_tool -id "/usr/local/share/vortex/modules/requests/lib/libssl.dylib" "modules/$1/lib/libssl.dylib"
    # sudo install_name_tool -change "/usr/local/lib/libcrypto.3.dylib" "/usr/local/share/vortex/modules/requests/lib/libcrypto.3.dylib" "modules/$1/lib/libssl.dylib"
    # CONFIG="-framework CoreFoundation -framework Security -L/usr/local/share/vortex/modules/requests/lib -I/usr/local/share/vortex/modules/requests/include"
    CONFIG="-framework CoreFoundation -framework Security"
elif [ "$1" = "sqlite" ]; then
    CONFIG="-lsqlite3"
else
    CONFIG=$CONFIG
fi

# clang++ \
# -dynamiclib \
# -g \
# -Ofast \
# -Wno-everything \
# -Imodules/"$1"/include \
# -Lmodules/"$1"/lib \
# $CONFIG \
# $LIBS \
# -std=c++20 \
# -stdlib=libc++ \
# "$PWD"/modules/"$1"/"$1".cpp \
# -o "$PWD"/modules/"$1"/bin/"$1" \
# -Wl,-rpath,@loader_path/../libs
# # -Wl,-rpath "$PWD"/modules/"$1"/lib

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
-Wl,-rpath,$RPATH