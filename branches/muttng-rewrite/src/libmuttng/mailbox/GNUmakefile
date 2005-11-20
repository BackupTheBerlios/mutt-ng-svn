FILES=mailbox remote_mailbox imap_mailbox nntp_mailbox pop3_mailbox local_mailbox
LIB=

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

test:
	@true

include $(CURDIR)/../../../GNUmakefile.compile_cpp
