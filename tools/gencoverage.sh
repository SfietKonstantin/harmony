#!/bin/sh
ROOTDIR=$(dirname $0)/..
cd $ROOTDIR

rm -rf report/
lcov -d . -c --initial -o lcov.da
lcov -d . -c -o lcov.da
lcov -r lcov.da "*moc*" -o lcov.da
lcov -r lcov.da "*qrc*" -o lcov.da
lcov -r lcov.da "*adaptor*" -o lcov.da
lcov -r lcov.da "*proxy*" -o lcov.da
lcov -r lcov.da "/usr/include/*" -o lcov.da
genhtml -o report --function-coverage -t "Harmony" --demangle-cpp lcov.da
rm lcov.da
lcov -z -d .