FILES=event
LIB=libevent.a

all: event.cpp event.h $(LIB)

event.cpp: EVENTS CONTEXTS event.pl event.cpp.in
	./event.pl

event.h: EVENTS CONTEXTS event.pl event.h.in
	./event.pl

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
