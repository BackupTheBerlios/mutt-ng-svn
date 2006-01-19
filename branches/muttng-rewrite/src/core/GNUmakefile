include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

FILES=mem exit str io conv hash list command sigs core net \
      buffer_base buffer_format buffer_token buffer_base64 buffer_qp
ifeq ($(WANT_LIBICONV),1)
CFLAGS+=$(CFLAGS_LIBICONV)
endif
ifneq ($(GETTEXTDIR),)
FILES+=intl
CFLAGS+=-DCORE_LOCALEDIR="\"$(CURDIR)/../po\""
endif
ifneq ($(LIBPCREDIR),)
FILES+=rx_pcre
else
FILES+=rx_posix
endif

CFLAGS+=$(CFLAGS_LIBIDN)
LIB:=libcore.a

SUBDIRS=test

all: $(LIB)

clean: subdirclean

test: subdirtest

include $(CURDIR)/../../GNUmakefile.compile_c
include $(CURDIR)/../../GNUmakefile.subdirs
