#!/bin/sh

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

LIBNAME=libengine

if [[ $1 == "rebuild" ]]; then
    echo "----------------------------------------"
    echo "  CLEANING "
    echo "----------------------------------------"

    xcodebuild -sdk iphoneos ARCHS="armv6 armv7" clean
    xcodebuild -sdk iphonesimulator ARCHS="i386" clean

fi

echo "----------------------------------------"
echo "  BUILDING  "
echo "----------------------------------------"

xcodebuild -sdk iphoneos ARCHS="armv6 armv7" build  || { echo "ERROR building arm"; exit 1; }
xcodebuild -sdk iphonesimulator ARCHS="i386" build || { echo "ERROR building i386"; exit 1; }


echo "----------------------------------------"
echo "  CREATING LIBRARY "
echo "----------------------------------------"

lipo -output ../$LIBNAME.a -create build/Release-iphoneos/$LIBNAME.a build/Release-iphonesimulator/$LIBNAME.a

echo
echo "Done"
echo
