#!/bin/sh


LIBS=""
if [ -z "$(ls -A modules/$1/lib)" ]; then
   LIBS=""
else
    LIBS="modules/$1/lib/lib*"
fi

clang++ \
-dynamiclib \
-g \
-Wno-everything \
-I modules/"$1"/include \
-L modules/"$1"/lib \
$LIBS \
-std=c++20 \
-stdlib=libc++ \
"$PWD"/modules/"$1"/"$1".cpp \
-o "$PWD"/modules/"$1"/bin/"$1"