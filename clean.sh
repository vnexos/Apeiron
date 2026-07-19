#!/bin/bash

rm -rf sysroot/
cmake --build build --target clean || true