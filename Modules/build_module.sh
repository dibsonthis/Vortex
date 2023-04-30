#!/bin/sh

clang++ \
-dynamiclib \
-O3 \
-Wno-everything \
-I modules/"$1"/include \
-L modules/"$1"/libraries \
-std=c++20 \
-stdlib=libc++ \
"$PWD"/modules/"$1"/"$1".cpp \
-o "$PWD"/modules/"$1"/bin/"$1"