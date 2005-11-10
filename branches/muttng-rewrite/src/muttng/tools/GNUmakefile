FILES=conf_tool mailx_tool muttng_tool sync_tool tool
LIB=libtool.a
BINARY=main
LINKLIBS=../ui/libui.a ../config/libconfig.a ../../libmuttng/libmuttng.a ../../core/libcore.a

all: $(OBJS) $(LIB) $(BINARY)

include ../../../GNUmakefile.config.default

CXXFLAGS+=-I.. -I../..

include ../../../GNUmakefile.compile_cpp
