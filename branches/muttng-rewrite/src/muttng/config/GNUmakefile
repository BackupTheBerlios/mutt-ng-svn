FILES=config abstract_command abstract_option \
      set_command string_option num_option bool_option quad_option url_option
LIB=libconfig.a

all: $(LIB) vars.h

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../.. -I../../libmuttng

vars.h: set_command.cpp config.pl
	./config.pl < set_command.cpp

include $(CURDIR)/../../../GNUmakefile.compile_cpp
