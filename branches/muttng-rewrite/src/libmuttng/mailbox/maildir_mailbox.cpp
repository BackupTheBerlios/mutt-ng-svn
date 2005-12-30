/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/maildir_mailbox.cpp
 * @brief Implementation: Maildir Mailbox base class
 */
#include "maildir_mailbox.h"
#include "libmuttng/config/config_manager.h"
#include <sys/types.h>
#include <dirent.h>
#include <cstring>
#include <iostream>
#include "core/str.h"

/** subdirs for Maildir */
static const char* dirs[] = { "/new", "/cur", "/tmp", NULL };

MaildirMailbox::MaildirMailbox(url_t* url_) : DirMailbox(url_),mNew(0),mTotal(0),mFlagged(0) {}

MaildirMailbox::~MaildirMailbox() {}

bool MaildirMailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void)msg;
  (void)dst;
  return false;
}

void MaildirMailbox::reg() {
  ConfigManager::regFeature("maildir");
}

bool MaildirMailbox::isMaildir (buffer_t* path, struct stat* st) {
  if (!S_ISDIR(st->st_mode)) return false;
  size_t l = path->len;
  unsigned short i = 0;
  while (dirs[i]) {
    buffer_shrink(path,l);
    buffer_add_str(path,dirs[i++],4);
    struct stat sb;
    if (stat(path->str,&sb)==0 && S_ISDIR(sb.st_mode)) {
      buffer_shrink(path,l);
      return true;
    }
  }
  return false;
}

void MaildirMailbox::parseFlags (const char* path) {
  char* p;
  if ((p = (char*)strchr(path,'.'))) {
    /* if filename contains '.', count as new */
    mNew++;
    mTotal++;
    if ((p = (char*)strchr(path,':')) && str_ncmp(p+1,"2,",2)==0 && (p+=2)) {
      /* if filename has flags, see which one */
      while (*p) {
        switch (*p) {
        case 'F': mFlagged++; break;
        case 'S': mNew--; break;
        default: break;
        }
        p++;
      }
    }
  }
}

mailbox_query_status MaildirMailbox::openMailbox() { return MQ_OK; }

mailbox_query_status MaildirMailbox::checkMailbox() {
  unsigned short i = 0;
  buffer_t path;
  buffer_init(&path);
  buffer_add_str(&path,url->path,-1);
  size_t l = path.len;
  /* don't go through $path/tmp==dirs[2] anymore */
  while (i<2) {
    DIR* dir;
    struct dirent* de;
    buffer_shrink(&path,l);
    buffer_add_str(&path,dirs[i],4);
    if ((dir = opendir (path.str))) {
      while ((de = readdir(dir)))
        if (*de->d_name != '.')
          parseFlags(de->d_name);
      closedir(dir);
    }
    i++;
  }
  buffer_free(&path);
  return MQ_OK;
}

unsigned long MaildirMailbox::msgNew() { return mNew; }
unsigned long MaildirMailbox::msgOld() { return 0; }
unsigned long MaildirMailbox::msgTotal() { return mTotal; }
unsigned long MaildirMailbox::msgFlagged() { return mFlagged; }
