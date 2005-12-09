include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

FILES=mailbox local_mailbox file_mailbox mbox_mailbox mmdf_mailbox dir_mailbox maildir_mailbox mh_mailbox
ifeq ($(WANT_REMOTE_MAILBOX),1)
FILES+=remote_mailbox
endif
ifeq ($(WANT_IMAP),1)
FILES+=imap_mailbox
endif
ifeq ($(WANT_NNTP),1)
FILES+=nntp_mailbox
endif
ifeq ($(WANT_POP3),1)
FILES+=pop3_mailbox
endif

LIB=

CXXFLAGS+=-I.. -I../..

test:
	@true

include $(CURDIR)/../../../GNUmakefile.compile_cpp
