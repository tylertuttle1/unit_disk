#!/bin/bash

if [ ! -d build ]; then mkdir build; fi

pushd build > /dev/null

g++ ../source/linux_main.cpp -std=c++11 -msse4.1 -O0 -g -ggdb -o linux_main
errorlevel=$?

# if the compiler succeeded, then run the output
if [ ${errorlevel} -eq 0 ]; then
    ./linux_main "$@"

    if [[ -v $WSLENV ]]; then
        "/mnt/c/Program Files/paint.net/PaintDotNet.exe" out.bmp
    else
        xdg-open out.bmp
    fi
fi

popd > /dev/null
