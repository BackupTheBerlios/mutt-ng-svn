FILES=mailbox \
      remote_mailbox imap_mailbox nntp_mailbox pop3_mailbox \
      local_mailbox file_mailbox mbox_mailbox mmdf_mailbox dir_mailbox maildir_mailbox mh_mailbox
LIB=

include $(CURDIR)/../../../GNUmakefile.whereis
include $(CURDIR)/../../../GNUmakefile.config.mine
include $(CURDIR)/../../../GNUmakefile.config

CXXFLAGS+=-I.. -I../..

test:
	@true

include $(CURDIR)/../../../GNUmakefile.compile_cpp
