#!/bin/bash

rm -rf build
mkdir -p build
cd build
cmake ../src && make -j4
echo "compile completed!!!"
