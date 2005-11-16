FILES=ui ui_curses ui_plain ui_text
LIB=libui.a

all: $(LIB)

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
