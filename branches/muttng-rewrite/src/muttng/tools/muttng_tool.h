/**
 * @ingroup muttng_bin
 * @addtogroup muttng_bin_muttng muttng(1)
 * @{
 * This class implements the interactive mail client.
 */
/**
 * @file muttng/tools/muttng_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: muttng(1)
 */
#ifndef MUTTNG_MUTTNG_TOOL_H
#define MUTTNG_MUTTNG_TOOL_H

#include "core/buffer.h"

#include "libmuttng/util/url.h"

#include "tool.h"

/**
 * @c muttng(1) class.
 */
class MuttngTool : public Tool {
  public:
    /** @copydoc Tool::Tool(). */
    MuttngTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~MuttngTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);

  private:
    /**
     * Dummy for testing connections without UI: Open folder.
     * @param folder Test Mailbox::fromURL() with this.
     * @bug remove this.
     */
    void doIndexMenu(const char* folder);
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

#endif /* !MUTTNG_MUTTNG_TOOL_H */

/** @} */
