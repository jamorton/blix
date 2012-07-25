#!/bin/sh

cd "$( dirname "${BASH_SOURCE[0]}" )/../engine"

LIBNAME=libengine

if [ ! -d "../libs/ios" ]; then
    mkdir -p ../libs/ios
fi

EXTRA=""
LIBPATH=Release
if [[ $1 == "debug" ]]; then
    EXTRA="-configuration Debug"
    LIBPATH=Debug
fi

if [[ $1 == "clean" ]]; then
    echo "----------------------------------------"
    echo "  CLEANING "
    echo "----------------------------------------"

    xcodebuild -sdk iphoneos ARCHS="armv6 armv7" clean
    xcodebuild -sdk iphonesimulator ARCHS="i386" clean
else
    echo "----------------------------------------"
    echo "  BUILDING  "
    echo "----------------------------------------"

    xcodebuild -sdk iphoneos ARCHS="armv6 armv7" build $EXTRA || { echo "ERROR building arm"; exit 1; }
    xcodebuild -sdk iphonesimulator ARCHS="i386" build $EXTRA || { echo "ERROR building i386"; exit 1; }


    echo "----------------------------------------"
    echo "  CREATING LIBRARY "
    echo "----------------------------------------"

    lipo -output ../libs/ios/$LIBNAME.a -create build/$LIBPATH-iphoneos/$LIBNAME.a build/$LIBPATH-iphonesimulator/$LIBNAME.a
fi

echo
echo "Done"
echo
