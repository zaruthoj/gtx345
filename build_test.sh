#!/bin/bash
GTEST_DIR=./gtest/googletest

g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
        -pthread -c ${GTEST_DIR}/src/gtest-all.cc
ar -rv libgtest.a gtest-all.o
g++ -std=c++11 -isystem ${GTEST_DIR}/include -pthread tests/listener_test.cpp listener.cpp libgtest.a \
    -o listener_test
