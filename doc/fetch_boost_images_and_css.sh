#!/bin/bash

set -e

BOOST_ROOT=~/boost/boost_1_61_0

cd html
cp -r ${BOOST_ROOT}/doc/src/* .
cd ..
