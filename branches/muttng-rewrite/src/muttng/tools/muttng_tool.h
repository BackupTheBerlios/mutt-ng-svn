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
    /**
     * Constructor for Tool::Tool().
     * @sa Tool::Tool().
     */
    MuttngTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~MuttngTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);

  private:
    void doIndexMenu(const char* folder);
    /**
     * Connected to Mailbox::sigGetUsername().
     * @param url URL to complete.
     * @return Yes/No.
     */
    bool getUsername (url_t* url);
    /**
     * Connected to Mailbox::sigGetPassword().
     * @param url URL to complete.
     * @return Yes/No.
     */
    bool getPassword (url_t* url);
};

#endif /* !MUTTNG_MUTTNG_TOOL_H */

/** @} */
