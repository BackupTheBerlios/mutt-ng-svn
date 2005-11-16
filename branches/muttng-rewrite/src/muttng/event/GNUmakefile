FILES=event
LIB=libevent.a

all: event.cpp event.h $(LIB)

event.cpp: EVENTS CONTEXTS event.pl event.cpp.in
	./event.pl

event.h: EVENTS CONTEXTS event.pl event.h.in
	./event.pl

include $(CURDIR)/../../../GNUmakefile.config.default

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
