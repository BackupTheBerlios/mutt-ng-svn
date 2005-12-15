/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/subject_header.h
 * @brief Subject MIME message header
 */
#ifndef LIBMUTTNG_MESSAGE_SUBJECT_HEADER_H
#define LIBMUTTNG_MESSAGE_SUBJECT_HEADER_H

#include "libmuttng/message/header.h"

/**
 * Subject header specialization. This is split from the generic
 * Header class to hide the regex stuff to hide the regular
 * expression stuff used to obtain the "real" subject. This class
 * transparently mangles Subject: headers to unify all reply indicators
 * into just <tt>Re:</tt>.
 */
class SubjectHeader : public Header {
  public:
    SubjectHeader(char * name = NULL, char * body = NULL);
    SubjectHeader(buffer_t * name, buffer_t * body);
    bool decode();
    bool encode();
    /** register module-specific config options */
    static void reg();
};

#endif /* !LIBMUTTNG_MESSAGE_HEADER_H */
