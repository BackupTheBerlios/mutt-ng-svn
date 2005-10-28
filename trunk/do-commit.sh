#!/bin/sh
svn update
echo "`svn info | awk -F': ' '/^Revision: / {print $2}'`+1" | bc > ./VERSION.svn
svn commit "$@"
