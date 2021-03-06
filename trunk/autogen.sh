#!/bin/sh --

if aclocal -I m4 && autoheader && (cd m4 && make -f Makefile.am.in ) \
  && automake --include-deps && autoconf
then
  echo
  echo "The configure script for Mutt-ng was successfully prepared."
  echo "Continue with \"./configure [your options]\"."
  echo
else
  echo
  echo "Some part of the preparation process failed."
  echo "Please refer to doc/devel-notes.txt for details."
  echo
  exit 1
fi
