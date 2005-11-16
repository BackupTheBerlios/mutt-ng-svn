FILES=config set_command abstract_command abstract_option string_option num_option bool_option
LIB=libconfig.a

all: $(LIB)

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
