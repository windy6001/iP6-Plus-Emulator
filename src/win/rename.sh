#!/bin/sh

for file in  `\find . -maxdepth 1 -name '*.c'`; do
   svn move $file ${file}pp
done
