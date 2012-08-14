#!/bin/sh

COUNT=$(cat /proc/cpuinfo | grep 'model name' | sed -e 's/.*: //' | wc -l)
echo "number of detected CPUs =" $COUNT

cppcheck -j $COUNT --force --inline-suppr . 2>errors.txt