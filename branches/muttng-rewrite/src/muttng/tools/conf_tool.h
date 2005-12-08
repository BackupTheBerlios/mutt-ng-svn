/**
 * @ingroup muttng_bin
 * @addtogroup muttng_bin_conf muttng-conf(1)
 * @{
 * This class implements a configuration diagnostics tool.
 */
/**
 * @file muttng/tools/conf_tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: muttng-conf(1)
 */
#ifndef MUTTNG_CONF_TOOL_H
#define MUTTNG_CONF_TOOL_H

#include "core/buffer.h"

#include "tool.h"

/**
 * @c muttng-conf(1) class.
 */
class ConfTool : public Tool {
  public:
    /** @copydoc Tool::Tool(). */
    ConfTool (int argc = 0, char** argv = NULL);
    /** destructor */
    ~ConfTool ();
    int main (void);
    const char* getName (void);
    void getUsage (buffer_t* dst);
  private:
    /** what to print */
    enum modes {
      /** print options */
      M_OPTS = 0,
      /** print bindings */
      M_BIND
    };
    /**
     * Handle default: print options.
     * @param annotated Whether to print default values and types as
     *                  comments: @c -a command line switch.
     * @param changed Whether to print only changed options:
     *                @c -D command line option.
     */
    void do_opts (bool annotated, bool changed);
    /**
     * Handle default: print bindings: @c -B command line option.
     * @param annotated Whether to print contexts, groups and
     *                  descriptions as comments: @c -a command line switch.
     * @param changed Whether to print only changed bindings:
     *                @c -D command line option.
     */
    void do_bind (bool annotated, bool changed);
};

#endif /* !MUTTNG_CONF_TOOL_H */

/** @} */
