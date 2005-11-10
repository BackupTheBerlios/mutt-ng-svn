/** @ingroup muttng */
/**
 * @file muttng/tools/conf_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief muttng(1) interface
 */
#ifndef MUTTNG_CONF_TOOL_H
#define MUTTNG_CONF_TOOL_H

#include "core/buffer.h"

#include "tool.h"

/**
 * @c muttng(1) class.
 */
class ConfTool : public Tool {
  public:
    /**
     * Constructor for Tool::Tool().
     * @sa Tool::Tool().
     */
    ConfTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~ConfTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);
};

#endif /* !MUTTNG_CONF_TOOL_H */
