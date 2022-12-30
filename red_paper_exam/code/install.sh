#!/bin/bash

mkdir -p build
cd build
cmake ../src && make -j8
echo "compile completed!!!"

