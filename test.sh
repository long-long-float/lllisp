#!/bin/sh

for file in `ls examples`; do
  path="examples/$file"
  echo
  echo "run $path"
  echo
  ./compile $path && ./a.out
done
