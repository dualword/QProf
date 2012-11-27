#!/bin/bash

set -e
 
export ARTISTIC_STYLE_OPTIONS="\
--mode=c \
--style=k&r \
--indent=spaces=4 \
--indent-classes \
--indent-switches \
--indent-preprocessor \
--break-blocks \
--pad-oper \
--add-brackets \
--convert-tabs \
--formatted \
--lineend=linux"

astyle $ARTISTIC_STYLE_OPTIONS $(find . -name "*.cpp")
astyle $ARTISTIC_STYLE_OPTIONS $(find . -name "*.c")
astyle $ARTISTIC_STYLE_OPTIONS $(find . -name "*.h")

rm -if $(find . -name "*.orig")
rm -if $(find . -name "*~")