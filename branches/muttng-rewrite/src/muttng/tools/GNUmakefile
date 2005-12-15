FILES=conf_tool mailx_tool muttng_tool sync_tool tool
LIB=libtool.a
BINARY=main
LINKLIBS=../muttng.o ../event/libevent.a ../ui/libui.a ../config/libconfig.a ../../libmuttng/libmuttng.a ../../core/libcore.a

all: $(OBJS) $(LIB) $(BINARY)
	ln -s main muttng || true
	ln -s main muttng-conf || true
	ln -s main muttng-sync || true
	ln -s main muttng-mailx || true

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
