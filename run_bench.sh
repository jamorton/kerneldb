#!/usr/bin/env bash

dbpath="$1"

if [ "$#" -ne 1 ]
then
    echo "Usage: ./run_bench.sh [db path]"
    exit 1
fi

make clean
make
sudo dd if=/dev/zero of=$dbpath count=1024 bs=1024
sudo rmmod krdb 2>&1 /dev/null
sudo insmod bin/krdb.ko
bin/krcl "$dbpath" bench
dmesg | tail -n 15
