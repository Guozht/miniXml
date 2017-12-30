#!/bin/bash


if [[ $1 == "clean" ]] ; then
  rm -rf bin
  exit 0
fi

if [[ !(-e bin) ]] ; then
  mkdir bin
fi


gcc -std=gnu11 src/*.c -g -c -fPIC -Wall -lbaselib -lpthread
if [[ $? != 0 ]] ; then
  exit $?
fi

mv *.o bin/
gcc bin/*.o -shared -o bin/libminixml.so

exit $?
