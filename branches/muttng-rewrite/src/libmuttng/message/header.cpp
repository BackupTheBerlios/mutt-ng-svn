/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/header.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief MIME message header
 */
#include "header.h"
#include <cstring>

Header::Header (char * name, char * body) {
  buffer_init(&this->name);
  buffer_init(&this->body);

  buffer_add_str(&this->name,name,-1);
  buffer_add_str(&this->body,body,-1);
}

Header::Header (buffer_t * name, buffer_t * body) {
  buffer_init(&this->name);
  buffer_init(&this->body);

  buffer_add_buffer(&this->name,name);
  buffer_add_buffer(&this->body,body);
}

bool Header::equalsName(char * name) {
  return 0==strcasecmp(name,this->name.str);
}

bool Header::equalsName(buffer_t * name) {
  return equalsName(name->str);
}

buffer_t * Header::getName() {
  return &name;
}

buffer_t * Header::getBody() {
  return &body;
}

void Header::setName(buffer_t * name) {
  buffer_shrink(&this->name,0);
  buffer_add_buffer(&this->name,name);
}

void Header::setBody(buffer_t * body) {
  buffer_shrink(&this->body,0);
  buffer_add_buffer(&this->body,body);
}

Header::~Header(void) {
  buffer_free(&name);
  buffer_free(&body);
}
