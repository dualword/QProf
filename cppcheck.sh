#!/bin/sh

cppcheck -j 2 --force --inline-suppr . 2>errors.txt