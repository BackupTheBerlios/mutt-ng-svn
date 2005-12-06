include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

FILES=connection plain_connection
ifeq ($(WANT_SSL_OPENSSL),1)
FILES+=ssl_connection
else
ifeq ($(WANT_SSL_GNUTLS),1)
FILES+=tls_connection
endif
endif
LIB=

CXXFLAGS+=-I.. -I../.. $(CXXFLAGS_SSL)

test:
	@true

include $(CURDIR)/../../../GNUmakefile.compile_cpp
