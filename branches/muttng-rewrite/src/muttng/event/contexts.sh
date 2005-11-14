#!/bin/sh

cat CONTEXTS | while read context
do
  echo "$context,"
  echo "\"$context\"," 1>&2
done 2>./contextlist.cpp 1>./contextlist.h
