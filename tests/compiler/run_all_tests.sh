#!/bin/sh

BIN_PATH=$1

for s in lexer parser semant cgen
do
  echo "Running test for $s"
  ./run_tests.pl $s $BIN_PATH || exit -1
done
