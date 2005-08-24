#!/bin/sh
date "+%Y%m%d" > ./VERSION.date
svn commit "$@"
