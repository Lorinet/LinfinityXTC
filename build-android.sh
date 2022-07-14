#!/bin/bash
./clean.sh
cd build
export NDK=/home/linfinity/Android/Sdk/ndk/24.0.8215888
cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=android-26 -DANDROID_ABI=arm64-v8a ..
make -j16