#!/bin/sh
chmod +x wiimake || exit 1
rm -rf cmake-build-debug
mkdir -p cmake-build-debug; cd cmake-build-debug
../wiimake ..
