#!/bin/bash

pushd build
cmake ..
make -j4
popd

