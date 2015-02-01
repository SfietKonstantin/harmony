#!/bin/sh
coffee -c *.coffee
pushd public > /dev/null 2>&1
coffee -c *.coffee
popd > /dev/null 2>&1
