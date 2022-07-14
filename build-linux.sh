#!/bin/bash
./clean.sh
cd build
cmake  ..
make -j16