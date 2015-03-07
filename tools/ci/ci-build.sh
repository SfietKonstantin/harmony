#!/bin/sh
set -e
mkdir -p build && cd build
qmake-qt5 -r CONFIG+=desktop CONFIG+=debug CONFIG+=testing ..
make -j11
