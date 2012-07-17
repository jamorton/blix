#!/bin/sh

cd "$( dirname "${BASH_SOURCE[0]}" )"
CPUCOUNT=$(./cpu-count.py)
cd ../engine/android

if [ ! -d "../../libs/android" ]; then
    mkdir -p ../../libs/android
fi

ndk-build -j $CPUCOUNT "$@"

cp -f obj/local/armeabi/*.a ../../libs/android
