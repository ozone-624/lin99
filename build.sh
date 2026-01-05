# !/bin/sh

rm -rf build
mkdir build && cd build
cmake -B . -S ..
cmake --build .