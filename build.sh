#!/bin/bash

if [[ !(-e bin) ]] ; then
  mkdir bin
fi

gcc -std=gnu11 -g src/*.c -Wall -lbaselib -lpthread -o bin/test.out
exit $?
