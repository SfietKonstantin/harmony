#!/bin/sh
DIRNAME=`dirname $0`
rm -f harmony-*.tgz > /dev/null 2>&1
pushd $DIRNAME/../node > /dev/null
npm pack
mv harmony-*.tgz ../rpm/harmony.tgz
popd > /dev/null
