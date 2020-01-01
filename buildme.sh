#!/bin/bash


# check lib and header file
HDRS_IN='/usr'
LIBS_IN='/usr'
find_dir_of_lib() {
    local lib=$(find ${LIBS_IN} -name "lib${1}.a" -o -name "lib${1}.$SO" 2>/dev/null | head -n1)
    if [ ! -z "$lib" ]; then
        dirname $lib
    fi
}
find_dir_of_header() {
    find -L ${HDRS_IN} -path "*/$1" | head -n1 | sed "s|$1||g"
}

#gtest
GTEST_LIB=$(find_dir_of_lib gtest)
if [ -z "$GTEST_LIB" ]; then
    echo "    \$(error \"Fail to find gtest lib\")"
    echo "sudo apt-get install -y cmake libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv libgtest* /usr/lib/ && cd -"
    exit 0
else
    GTEST_HDR=$(find_dir_of_header gtest/gtest.h)
    if [ -z "$GTEST_HDR" ]; then
        echo "    \$(error \"Fail to find gtest include\")"
        echo "sudo apt-get install -y cmake libgtest-dev && cd /usr/src/gtest && sudo cmake . && sudo make && sudo mv libgtest* /usr/lib/ && cd -"
        exit 0
    fi
fi

echo $GTEST_LIB "    "
echo $GTEST_HDR "    "

make
