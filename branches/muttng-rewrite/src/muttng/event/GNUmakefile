FILES=event
LIB=libevent.a

all: eventlist.h eventlist.cpp contextlist.h contextlist.cpp $(LIB)

eventlist.%: EVENTS
	./events.sh

contextslist.%: CONTEXTS
	./contexts.sh

include $(CURDIR)/../../../GNUmakefile.config.default

CXXFLAGS+=-I.. -I../..

include $(CURDIR)/../../../GNUmakefile.compile_cpp
