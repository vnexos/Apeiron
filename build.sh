#!/bin/bash

ARCH=$1

if [ -z "$ARCH" ]; then
    ARCH=x86_64
fi

make ARCH=$ARCH clean

make ARCH=$ARCH
