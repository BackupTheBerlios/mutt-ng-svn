/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/subject_header.cpp
 * @brief Generic MIME message header
 */
#include "subject_header.h"

#include "libmuttng/config/config_manager.h"

#include "core/rx.h"

static rx_t* ReplyRegex = NULL;

SubjectHeader::SubjectHeader(char* name, char* body) : Header(name,body) {}

SubjectHeader::SubjectHeader(buffer_t* name, buffer_t* body) : Header(name,body) {}

bool SubjectHeader::encode() { return true; }
bool SubjectHeader::decode() { return true; }

void SubjectHeader::reg() {
  ConfigManager::reg(new RXOption("reply_regexp","^(re([\\[0-9\\]+])*|aw):[ \t]*",&ReplyRegex));
}
