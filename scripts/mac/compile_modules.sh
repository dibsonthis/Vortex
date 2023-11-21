!/bin/sh

LIBS=""

rm -r bin/build/modules/mac
mkdir -p bin/build/modules/mac
cd Modules/modules
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
    LIBS="$FILE/lib/lib*"
fi

CONFIG=""

if [ "$FILE" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL $FILE/include/SDL2/*.cpp"
elif [ "$FILE" = "requests" ]; then
    CONFIG="-framework CoreFoundation -framework Security"
elif [ "$FILE" = "sqlite" ]; then
    CONFIG="-lsqlite3"
else
    CONFIG=$CONFIG
fi

clang++ \
-fPIC \
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

cp -r Modules/modules bin/build/modules/mac

cd bin/build/modules/mac

for FILE in *;
do

rm -r $FILE/include
rm "$FILE/$FILE.cpp"

done