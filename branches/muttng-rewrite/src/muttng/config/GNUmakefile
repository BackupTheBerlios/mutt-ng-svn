FILES=config set_command abstract_command abstract_option string_option num_option bool_option
LIB=libconfig.a

all: $(LIB) vars.h

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

vars.h: set_command.cpp config.pl
	./config.pl < set_command.cpp

include $(CURDIR)/../../../GNUmakefile.compile_cpp
