#!/bin/sh

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
#--brackets=attach \
--convert-tabs \
--lineend=linux"

astyle $(find . -name "*.cpp")
astyle $(find . -name "*.h")

rm -if $(find . -name "*.orig")
rm -if $(find . -name "*~")