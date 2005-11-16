FILES:=buffer mem exit str io conv hash list
LIB:=libcore.a

SUBDIRS=test

all: $(LIB)

clean: subdirclean

test: subdirtest

include $(CURDIR)/../../GNUmakefile.whereis
include $(CURDIR)/../../GNUmakefile.config.mine
include $(CURDIR)/../../GNUmakefile.config
include $(CURDIR)/../../GNUmakefile.compile_c
include $(CURDIR)/../../GNUmakefile.subdirs
