#!/bin/sh
svn update
echo "`svn info | awk -F': ' '/^Revision: / {print $2}'`+1" | bc > ./VERSION.svn
if [ x"$1" != x ] ; then
  svn commit do-commit.sh VERSION.svn "$@"
else
  svn commit
fi
