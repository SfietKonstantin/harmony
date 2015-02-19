#!/bin/bash
ROOTDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

eval `dbus-launch --sh-syntax`
echo "Using DBUS environnement variable: $DBUS_SESSION_BUS_ADDRESS"

pushd build/src/tests/unit/ > /dev/null
for d in $(ls -d tst*); do
    if [ -d $d ]; then
        pushd $d > /dev/null
        echo "Running $d"
        ./$d -xml > ../$d-unformatted.xml
        # apply xslt
        echo "Adapting results for $d"
        xsltproc $ROOTDIR/qtestxml2junitxml.xsl ../$d-unformatted.xml > ../$d.xml
        rm ../$d-unformatted.xml
        popd > /dev/null
    fi
done
popd > /dev/null
pushd tools > /dev/null
./gencoverage.sh
popd > /dev/null