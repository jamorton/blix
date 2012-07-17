#!/bin/sh

if [ -z "$1" ]; then
    echo "Usage: build-dep-ios [dep_folder]"
    exit 0
fi

cd "$( dirname "${BASH_SOURCE[0]}" )/../deps"

if [ ! -d "$1" ]; then
    echo "Dependency $1 not found."
    exit 1
fi

if [ ! -d "../libs/ios" ]; then
    mkdir -p  ../libs/ios
fi

LIBNAME="lib"$1

if [[ $1 == lib* ]]; then
    LIBNAME=$1
fi

cd $1

if [[ $2 == "clean" ]]; then
    echo "----------------------------------------"
    echo "  CLEANING "
    echo "----------------------------------------"

    xcodebuild -sdk iphoneos ARCHS="armv6 armv7" clean
    xcodebuild -sdk iphonesimulator ARCHS="i386" clean
else
    echo "----------------------------------------"
    echo "  BUIDLING "
    echo "----------------------------------------"

    xcodebuild -sdk iphoneos ARCHS="armv6 armv7" build || { echo "ERROR building arm"; exit 1; }
    xcodebuild -sdk iphonesimulator ARCHS="i386" build || { echo "ERROR building i386"; exit 1; }

    echo "----------------------------------------"
    echo "  CREATING LIBRARY "
    echo "----------------------------------------"

    lipo -output ../../libs/ios/$LIBNAME.a -create build/Release-iphoneos/$LIBNAME.a build/Release-iphonesimulator/$LIBNAME.a
fi

echo
echo "Done."
