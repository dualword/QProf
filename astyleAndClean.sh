#!/bin/sh

astyle $(find . -name "*.cpp")
astyle $(find . -name "*.h")

rm -if $(find . -name "*.orig")
rm -if $(find . -name "*~")