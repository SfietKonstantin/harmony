#!/bin/sh
set -e
mkdir -p build && cd build
qmake-qt5 -r CONFIG+=desktop CONFIG+=debug CONFIG+=harmonydebug ..
make -j11
