#!/usr/bin/env bash
cd ../
gcc -o t test.c -lpthread
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
   ./t
    sleep 2
    rm -rf t
elif [[ "$unamestr" == 'MINGW64_NT-10.0' ]]; then
    ./t.exe
    sleep 2
    rm -rf t.exe
fi