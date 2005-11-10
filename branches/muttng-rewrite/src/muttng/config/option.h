/**
 * @file muttng/config/option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Configuration option definition
 */
#ifndef MUTTNG_CONFIG_OPTION_H
#define MUTTNG_CONFIG_OPTION_H

/**
 * This is what internally an option looks like
 * @todo In fact this should to into set_command.cpp as that's the only
 * place where it's needed. However, we derive StringOption and others
 * from AbstractOption so we need it there, too...
 */
typedef struct option_t {
  /** type, see SetCommand::types */
  int type;
  /** variable name */
  const char* name;
  /** initial value */
  const char* init;
  /** pointer to storage for content, ID or whatever */
  unsigned long data;
} option_t;

#endif /* !MUTTNG_CONFIG_OPTION_H */
