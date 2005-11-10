/** @ingroup muttng */
/**
 * @file muttng/tools/mailx_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief muttng(1) interface
 */
#ifndef MUTTNG_MAILX_TOOL_H
#define MUTTNG_MAILX_TOOL_H

#include "core/buffer.h"

#include "tool.h"

/**
 * @c muttng(1) class.
 */
class MailxTool : public Tool {
  public:
    /**
     * Constructor for Tool::Tool().
     * @sa Tool::Tool().
     */
    MailxTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~MailxTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);
};

#endif /* !MUTTNG_MAILX_TOOL_H */
