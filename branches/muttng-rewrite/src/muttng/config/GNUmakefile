FILES=config set_command abstract_command abstract_option string_option
LIB=libconfig.a

all: $(LIB)

include $(CURDIR)/../../../GNUmakefile.config.default

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
