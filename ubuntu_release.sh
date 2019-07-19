#!/bin/bash

set -e

releases="xenial disco cosmic bionic trusty" # List of releases to generate

if [ ! -f "bitwise-v$1.tar.gz" ]; then
   wget https://github.com/mellowcandle/bitwise/releases/download/v$1/bitwise-v$1.tar.gz
fi

for release in $releases; do
    echo "Creating release for $release"
    mkdir -p $release
    cd $release
    rm -rf bitwise-v$1
    tar xf ../bitwise-v$1.tar.gz
    cd bitwise-v$1
    cp -r ../../debian .
    dch -D $release -v $1-1ubuntu+$release v$1
    dpkg-buildpackage -S
    cd ..
    dput ppa bitwise_$1-1ubuntu+${release}_source.changes
    cd ../
done


