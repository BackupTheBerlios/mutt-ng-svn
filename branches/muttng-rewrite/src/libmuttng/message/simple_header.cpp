/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/simple_header.cpp
 * @brief Generic MIME message header
 */
#include "simple_header.h"

SimpleHeader::SimpleHeader(char* name, char* body) : Header(name,body) {}

SimpleHeader::SimpleHeader(buffer_t* name, buffer_t* body) : Header(name,body) {}

bool SimpleHeader::encode() { return true; }
bool SimpleHeader::decode() { return true; }
