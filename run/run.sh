#!/usr/bin/env bash
cd ../
gcc -o pmm parallel_mat_mul.c -lpthread
unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
   ./pmm
    sleep 2
    rm -rf pmm
elif [[ "$unamestr" == 'MINGW64_NT-10.0' ]]; then
    ./pmm.exe
    sleep 2
    rm -rf pmm.exe
fi