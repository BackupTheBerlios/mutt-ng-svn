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

  if (name)
    buffer_add_str(&this->name,name,-1);

  if (body)
    buffer_add_str(&this->body,body,-1);
}

Header::Header (buffer_t * name, buffer_t * body) {
  buffer_init(&this->name);
  buffer_init(&this->body);

  if (name)
    buffer_add_buffer(&this->name,name);

  if (body)
    buffer_add_buffer(&this->body,body);
}

bool Header::equalsName(char * name) {
  if (!name) return false;
  return 0==strcasecmp(name,this->name.str);
}

bool Header::equalsName(buffer_t * name) {
  if (!name) return false;
  return equalsName(name->str);
}

buffer_t * Header::getName() {
  return &name;
}

buffer_t * Header::getBody() {
  return &body;
}

void Header::setName(buffer_t * name) {
  if (name) {
    buffer_shrink(&this->name,0);
    buffer_add_buffer(&this->name,name);
  }
}

void Header::setBody(buffer_t * body) {
  if (body) {
    buffer_shrink(&this->body,0);
    buffer_add_buffer(&this->body,body);
  }
}

buffer_t * operator<<(buffer_t * buf, Header & h) {
  if (buf) {
    buffer_add_buffer(buf,h.getName());
    buffer_add_str(buf,": ",2);
    buffer_add_buffer(buf,h.getBody());
    buffer_add_str(buf,"\r\n",2);
  }
  return buf;
}

Header::~Header(void) {
  buffer_free(&name);
  buffer_free(&body);
}
