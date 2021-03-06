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
 *
 * This file is published under the GNU General Public License.
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
};

#endif /* !MUTTNG_MUTTNG_TOOL_H */

/** @} */
