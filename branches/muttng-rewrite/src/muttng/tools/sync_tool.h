/** @ingroup muttng */
/**
 * @file muttng/tools/sync_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief muttng(1) interface
 */
#ifndef MUTTNG_SYNC_TOOL_H
#define MUTTNG_SYNC_TOOL_H

#include "core/buffer.h"

#include "tool.h"

/**
 * @c muttng(1) class.
 */
class SyncTool : public Tool {
  public:
    /**
     * Constructor for Tool::Tool().
     * @sa Tool::Tool().
     */
    SyncTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~SyncTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);
};

#endif /* !MUTTNG_SYNC_TOOL_H */
