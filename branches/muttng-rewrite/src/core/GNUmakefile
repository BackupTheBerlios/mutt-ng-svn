include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config

FILES:=buffer mem exit str io conv hash list base64 qp rx command sigs core
ifeq ($(WANT_LIBICONV),1)
CFLAGS+=$(CFLAGS_LIBICONV)
endif
LIB:=libcore.a

SUBDIRS=test

all: $(LIB)

clean: subdirclean

test: subdirtest

include $(CURDIR)/../../GNUmakefile.compile_c
include $(CURDIR)/../../GNUmakefile.subdirs
