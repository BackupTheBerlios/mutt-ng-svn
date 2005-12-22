include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

FILES=buffer mem exit str io conv hash list base64 qp rx command sigs core net
ifeq ($(WANT_LIBICONV),1)
CFLAGS+=$(CFLAGS_LIBICONV)
endif
ifneq ($(GETTEXTDIR),)
FILES+=intl
CFLAGS+=-DCORE_LOCALEDIR="\"$(CURDIR)/../po\""
endif
CFLAGS+=$(CFLAGS_LIBIDN)
LIB:=libcore.a

SUBDIRS=test

all: $(LIB)

clean: subdirclean

test: subdirtest

include $(CURDIR)/../../GNUmakefile.compile_c
include $(CURDIR)/../../GNUmakefile.subdirs
