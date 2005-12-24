/**
 * @ingroup muttng_bin
 * @addtogroup muttng_bin_query muttng-query(1)
 * @{
 * This class implements the interactive mail client.
 */
/**
 * @file muttng/tools/query_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: muttng-query(1)
 */
#ifndef MUTTNG_QUERY_TOOL_H
#define MUTTNG_QUERY_TOOL_H

#include "core/buffer.h"

#include "libmuttng/util/url.h"
#include "libmuttng/mailbox/mailbox.h"

#include "tool.h"

/**
 * @c muttng(1) class.
 */
class QueryTool : public Tool {
  public:
    /** @copydoc Tool::Tool(). */
    QueryTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~QueryTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);
  private:
    /** modes we know */
    enum modes {
      /** default: query given folders */
      M_MAILBOX_QUERY = 0,
      /** with -i: turn given hosts into IDN */
      M_IDN,
      /** print mime charsets */
      M_CHARSET_MIME,
      /** print charset aliases */
      M_CHARSET_ALIAS
    };
    /** whether to print in colon-mode */
    bool colon;
    /** whether to not print messages */
    bool quiet;
    /**
     * Print folder stats.
     * @param folder Folder.
     */
    void folderStats(Mailbox* folder);
    /** testing mailbox login. */
    void doURLs();
    /** testing IDN conversions. */
    void doIDN();
    /**
     * Print charsets.
     * @param mime Whether to print MIME or aliases.
     */
    void doCharsets (bool mime);
    /**
     * Dummy for testing connections without UI: Connected to Mailbox::sigGetUsername().
     * @param url URL to complete.
     * @return Yes/No.
     * @bug remove this.
     */
    bool getUsername (url_t* url);
    /**
     * Dummy for testing connections without UI: Connected to Mailbox::sigGetPassword().
     * @param url URL to complete.
     * @return Yes/No.
     * @bug remove this.
     */
    bool getPassword (url_t* url);
};

#endif /* !MUTTNG_QUERY_TOOL_H */

/** @} */
