#!/bin/sh

LIBS=""

cd modules
for FILE in *; 
do

echo Compiling $FILE

if [ -z "$(ls -A $FILE/lib)" ]; then
   LIBS=""
else
    LIBS="$FILE/lib/lib*"
fi

clang++ \
-dynamiclib \
-O3 \
-Wno-everything \
-I $FILE/include \
-L $FILE/lib \
$LIBS \
-std=c++20 \
-stdlib=libc++ \
"$PWD"/$FILE/$FILE.cpp \
-o "$PWD"/$FILE/bin/$FILE \
-Wl,-rpath "$PWD"/$FILE/lib
done