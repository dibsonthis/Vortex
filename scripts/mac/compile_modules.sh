!/bin/sh

LIBS=""

rm -r bin/build/modules/mac
mkdir -p bin/build/modules/mac
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
    LIBS="$FILE/lib/lib*"
fi

CONFIG=""

if [ "$FILE" = "sdl" ]; then
    CONFIG="-D_THREAD_SAFE -framework GLUT -framework OpenGL $FILE/include/SDL2/*.cpp -lSDL_image"
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

popd

cp -r Modules/modules bin/build/modules/mac

pushd "bin/build/modules/mac"

echo $PWD

for _FILE in *;
do

rm -r $_FILE/include
rm "$_FILE/$_FILE.cpp"

done

popd