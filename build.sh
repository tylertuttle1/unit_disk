#!/bin/bash

cd build

g++ ../source/linux_main.cpp -msse4.1 -O0 -g -ggdb -o linux_main
errorlevel=$?

# if the compiler didn't error, run the program
if [ ${errorlevel} -eq 0 ]; then
    ./linux_main
    "/mnt/c/Program Files/paint.net/PaintDotNet.exe" out.bmp
fi

cd ..
