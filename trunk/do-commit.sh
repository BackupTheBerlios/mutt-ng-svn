#!/bin/sh
svn update
CURREV=`svn info | grep '^Revision' | cut -d : -f 2`
echo "`echo $CURREV+1 | bc`" > ./VERSION.date
svn commit "$@"
