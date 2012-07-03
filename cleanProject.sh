#!/bin/sh

make clean
rm -if $(find . -name "*.orig")
rm -if $(find . -name "*~")
rm -if $(find ./sources/ -name "*.cmake")
rm -if $(find . -name "Makefile")
rm -ifr $(find . -name "CMakeFiles")

rm ./*.cmake
rm ./CMakeCache.txt
