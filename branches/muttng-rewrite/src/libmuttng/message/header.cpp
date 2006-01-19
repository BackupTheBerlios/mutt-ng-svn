/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/header.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief MIME message header
 *
 * This file is published under the GNU General Public License.
 */
#include "header.h"
#include <cstring>
#include <iostream>

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

void Header::serialize(buffer_t * buf) {
  if (buf) {
    buffer_add_buffer(buf,getName());
    buffer_add_str(buf,": ",2);
    buffer_add_buffer(buf,getBody());
  }
}

void Header::parseLine(buffer_t * buf) {
  buffer_t name, body;
  if (buf) {
    buffer_chomp(buf);

    char * sep = strchr(buf->str,':');

    if (!sep) {
      /* when no separator could be found, simply use the complete buffer as header name, and set an empty header body */
      setName(buf);

      buffer_init(&body);
      setBody(&body);
      buffer_free(&body);
    } else {
      /* set header name */
      buffer_init(&name);
      buffer_add_str(&name,buf->str,sep - buf->str);
      setName(&name);
      buffer_free(&name);

      ++sep;
      while (*sep != '\0' && *sep == ' ') ++sep;

      /* set header body */
      buffer_init(&body);
      buffer_add_str(&body,sep,-1);
      setBody(&body);
      buffer_free(&body);
    }
  }
}

Header::~Header(void) {
  buffer_free(&name);
  buffer_free(&body);
}
