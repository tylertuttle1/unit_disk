#!/bin/bash

if [[ ! -d build ]]; then mkdir build; fi
pushd build > /dev/null

./linux_main "$@"
if [[ -v $WSLENV ]]; then
    "/mnt/c/Program Files/paint.net/PaintDotNet.exe" out.bmp
else
    xdg-open out.bmp
fi

popd > /dev/null
