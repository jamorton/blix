#!/bin/sh

if [ -z "$1" ]; then
    echo "Usage: ./build_dep [dep_folder]"
    exit 0
fi

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ ! -d "$1" ]; then
    echo "Dependency $1 not found."
    exit 1
fi

LIBNAME="lib"$1

if [[ $1 == lib* ]]; then
    LIBNAME=$1
fi

cd $1

if [[ $2 == "rebuild" ]]; then
    echo "----------------------------------------"
    echo "  CLEANING "
    echo "----------------------------------------"

    xcodebuild -sdk iphoneos ARCHS="armv6 armv7" clean
    xcodebuild -sdk iphonesimulator ARCHS="i386" clean
fi

echo "----------------------------------------"
echo "  BUIDLING "
echo "----------------------------------------"

xcodebuild -sdk iphoneos ARCHS="armv6 armv7" build || { echo "ERROR building arm"; exit 1; }
xcodebuild -sdk iphonesimulator ARCHS="i386" build || { echo "ERROR building i386"; exit 1; }

echo "----------------------------------------"
echo "  CREATING LIBRARY "
echo "----------------------------------------"

lipo -output ../$LIBNAME.a -create build/Release-iphoneos/$LIBNAME.a build/Release-iphonesimulator/$LIBNAME.a
