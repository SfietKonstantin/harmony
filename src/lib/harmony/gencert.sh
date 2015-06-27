#!/bin/sh

if [ -z $1 ]; then
    echo "Please provide a path"
    exit 1
fi

if [ -d $1 ]; then
    pushd $1 > /dev/null
    rm *.crt
    rm *.pub
    rm *.key
    rm *.pem
    popd > /dev/null
    rmdir $1
fi

mkdir -p $1
pushd $1 > /dev/null

# CA
openssl genrsa -des3 -out harmony-ca.key -passout pass:harmony-ca-key 1024
openssl req -new -key harmony-ca.key -out harmony-ca.csr -passin pass:harmony-ca-key -subj '/C=FR/ST=Ile de France/L=Paris/CN=Harmony project'
openssl x509 -req -days 3650 -in harmony-ca.csr -signkey harmony-ca.key -out harmony-ca.crt -passin pass:harmony-ca-key

# Server
openssl genrsa -des3 -out harmony.key -passout pass:harmony-key 1024
openssl req -new -key harmony.key -out harmony.csr -passin pass:harmony-key -subj '/C=FR/ST=Ile de France/L=Paris/CN=Harmony project'  > /dev/null 2>&1
cp harmony.key harmony.key.orig
openssl rsa -in harmony.key.orig -out harmony.key -passin pass:harmony-key
openssl x509 -req -days 3650 -in harmony.csr -signkey harmony.key -out harmony.crt -passin pass:harmony-key

cp harmony.crt harmony.pem
cat harmony.key >> harmony.pem

# Public 
openssl rsa -in harmony.key -pubout -out harmony-key.pub

# Cleanup
rm harmony-ca.key
rm harmony.key.orig
rm *.csr

popd > /dev/null
