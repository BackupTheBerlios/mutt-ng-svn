#!/bin/sh

# ./configure --enable-pgp --enable-smime --with-regex --enable-pop --enable-imap --enable-nntp --enable-imap-edit-threads --with-ssl --enable-hcache --with-libesmtp && make
if [ `uname -s` = 'Darwin' ] ; then
  export CFLAGS="-I/sw/include"
  export LDFLAGS="-L/sw/lib"
fi
./configure --enable-pgp --enable-smime --with-regex --enable-pop --enable-imap --enable-nntp --enable-imap-edit-threads --with-ssl --enable-hcache && make
