#!/bin/bash

ARCH=$1

if [ -z "$ARCH" ]; then
    ARCH=x86_64
fi

./build.sh $ARCH

make ARCH=$ARCH run
