#!/bin/bash
compiler=$(which clang++)
if [[ $? != 0 ]]
  then
  echo "Need clang++ with c++11 std support, exiting"
  exit 2
fi

compilerOps='-std=c++11 -pthread'
debugFlags=""
if [[ $# == 1 ]]
  then
  if [[ $1 == "-debug=true" ]]
    then
    debugFlags='-g -fsanitize=address -fno-omit-frame-pointer'
  fi
fi

clang++ ${compilerOps} ${debugFlags} src/console.cpp src/main.cpp example/example.cpp -o test-out