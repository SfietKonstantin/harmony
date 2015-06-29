#!/bin/sh

# Build client files
pushd public > /dev/null
coffee -c *.coffee

pushd modules > /dev/null
for d in *; do
    if [ -d "$d" ]; then
        pushd $d > /dev/null
        coffee -c *.coffee
        popd > /dev/null
    fi
done
popd > /dev/null

popd > /dev/null
