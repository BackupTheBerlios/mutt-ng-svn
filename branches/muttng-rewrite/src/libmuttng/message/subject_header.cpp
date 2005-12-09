/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/subject_header.cpp
 * @brief Generic MIME message header
 */
#include "subject_header.h"

SubjectHeader::SubjectHeader(char* name, char* body) : Header(name,body) {}

SubjectHeader::SubjectHeader(buffer_t* name, buffer_t* body) : Header(name,body) {}

bool SubjectHeader::encode() { return true; }
bool SubjectHeader::decode() { return true; }
