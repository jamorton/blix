#!/bin/sh

ndk-build -j8 || { echo "NDK build failed"; exit 1; }

rm -rf assets
cp -r ../assets .

ant debug || { echo "APK build failed "; exit 1; }

adb install -r bin/BlixTests-debug.apk
