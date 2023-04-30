#!/bin/sh

cd modules
for FILE in *; 
do 
clang++ \
-dynamiclib \
-O3 \
-Wno-everything \
-I $FILE/include \
-L $FILE/libraries \
-std=c++20 \
-stdlib=libc++ \
"$PWD"/$FILE/$FILE.cpp \
-o "$PWD"/$FILE/bin/$FILE
done