#!/bin/sh

cat EVENTS | while read event
do
  echo "$event,"
  echo "\"$event\"," 1>&2
done 2>./eventlist.cpp 1>./eventlist.h
