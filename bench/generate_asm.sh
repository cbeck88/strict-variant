#!/bin/bash

FLAGS="-O3 -S -DNUM_VARIANTS=2 -DSEQ_LENGTH=1000 -DREPEAT_NUM=10000 -DRNG_SEED=42"

if [[ -z ${CXX} ]]; then echo "Must set CXX"; exit 1; fi

echo "CXX = " ${CXX}

${CXX} -Iinclude/ -I../include/ -nostdinc++ -Ilibcxx-variant/include/ -std=c++1z ${FLAGS} std_variant.cpp
${CXX} -Iinclude/ -I../include/                           -std=c++11 ${FLAGS} safe_variant.cpp
${CXX} -Iinclude/ -I/usr/include/                         -std=c++11 ${FLAGS} boost_variant.cpp
${CXX} -Iinclude/ -Impark_variant/include/                -std=c++14 ${FLAGS} exp_variant.cpp
