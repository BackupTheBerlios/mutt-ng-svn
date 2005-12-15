/**
 * @file muttng/config/config.h
 * @author Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Configuration parser
 */
#ifndef MUTTNG_CONFIG_CONFIG_H
#define MUTTNG_CONFIG_CONFIG_H

#include "../muttng.h"

#include "core/buffer.h"

/**
 * Config parser class.
 * Actually this needs to be a class so it inherits debug support.
 */
class Config : public Muttng {
  public:
    /**
     * Read complete config.
     * @param error Mandatory error message buffer.
     * @return Success.
     */
    bool read(buffer_t* error);
    /**
     * Globally register all options.
     */
    static void reg();
  private:
    /**
     * Read configuration file.
     * @param path Filename or command.
     * @param error Mandatory error message buffer.
     * @return Success.
     */
    bool config_read_file(buffer_t* path, buffer_t* error);
};

#endif /* !MUTTNG_CONFIG_CONFIG_H */
