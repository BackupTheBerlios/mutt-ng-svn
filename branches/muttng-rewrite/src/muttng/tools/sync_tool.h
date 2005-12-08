/**
 * @ingroup muttng_bin
 * @addtogroup muttng_bin_sync muttng-sync(1)
 * @{
 * This class implements a bidirectional syncer between any two
 * supported mailboxes.
 */
/**
 * @file muttng/tools/sync_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: muttng-sync(1)
 */
#ifndef MUTTNG_SYNC_TOOL_H
#define MUTTNG_SYNC_TOOL_H

#include "core/buffer.h"

#include "tool.h"

/**
 * @c muttng-sync(1) class.
 */
class SyncTool : public Tool {
  public:
    /** @copydoc Tool::Tool(). */
    SyncTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~SyncTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);
};

#endif /* !MUTTNG_SYNC_TOOL_H */

/** @} */
