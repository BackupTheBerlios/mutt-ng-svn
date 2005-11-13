/* vim:foldmethod=marker:foldlevel=0
 */
/* doxygen documentation {{{ */
/**
 * @ingroup muttng
 * @addtogroup muttng_event Event Handling
 * @{
 *
 * @section muttng_event_intro Introduction
 *
 *   Event handling is most the most critical part as its where the
 *   important logic is done somehow. As there's no real implementation
 *   here yet, this is a rough description of how it's supposed to work
 *   some day.
 *
 *   There're internally two tables:
 *
 *     -# The first multi-dimensional table maps an event of a context
 *        to a list of callback functions.
 *     -# The second one maps a key or a sequence to types of events we
 *        know. User interfaces will be responsible to get key sequences
 *        and queue them in.
 *
 * @section muttng_event_ctx Contexts
 *
 *   A context is what the former menus or screeens were. Internally
 *   there will be a stack listing every context in the order it
 *   appeared. When entering a new context, it'll be pushed; when
 *   eventually really leaving it, it'll be poped.
 *
 *   The idea is this:
 *   when entering the index, an ID for the index menu context will be
 *   pushed on the stack. After doing something, the user may request to
 *   call up the help screen. This context will also be pushed so that
 *   there're now two on the stack. When we receive a key, we take the
 *   topmost context of the stack to lookup the event to tigger: for the
 *   help context in this case. After leaving the help screen, the help
 *   context will be poped so that we're back with the index context
 *   being topmost so that every key now applies to the index context.
 *
 *   Every context pop and push will trigger a leave or enter event to
 *   which internal functions can be bound. For example, when entering
 *   the index, we will catch the tiggered event to do sorting of the
 *   mailbox prior to call the UI for the index. Contexts must be
 *   organized in a stack since single context switches would be evil:
 *   due to the stack, we can say whether we really leave the index menu
 *   to catch the leave event for syncing changes, moving messages, etc.
 *   With a single "currentContext", we would have to do this for the
 *   context switch to the help screen of the above example.
 *
 *   All former hooks will be implemented this way: just a function
 *   catching the enter event.
 *
 * @section muttng_event_keys Key Bindings
 *
 *   As our goal is to fully separate the UI from mail and its logic,
 *   the key bindings are somewhat independent from the UI.
 *
 *   When reading the configuration and we get @c bind and @c macro
 *   commands, we bind them internally to their functions. The bridge to
 *   the UI is established via the context table since after looking up
 *   the event to trigger for a key and context, the UI can do
 *   everything it wants with it. The other way will work too: UIs can
 *   have methods to obtain keys we don't know details about (example: a
 *   GUI menu which calls a previously setup callback function) and
 *   request to queue them. We'll perform event lookup and emit them.
 *
 * @section muttng_event_event Events
 *
 *   There's a number of events, formerly all the @c OP_* and more.
 *
 * @section muttng_event_drawback Examples and consequences
 *
 *   This approach has some drawbacks.
 *
 *   For example, the event handler doesn't know anything about the UI
 *   and how it gets keys. For curses this is @c getch() so we could
 *   simply write a wrapper around which simulates keys. However, this
 *   won't work with a GUI which has a more or less different method,
 *   i.e. it would be difficult to push a keystroke into a GUI library.
 *   Thus, for macros and such we can't any longer allow "raw" keys like
 *   for return, etc. but only function names.
 *
 *   The following won't work any longer:
 *
 *   <pre>
 * macro pager \cx ,r13on</pre>
 *
 *    (which is the entry point for a toggle-rot13-macro) because we can't
 *    make a GUI believe it got the key sequence <code>,r13on</code>
 *    which itself is bound to a macro. This is a simple example with only one key
 *    sequence, more complicated examples are possible of course. With
 *    the new type of event handling, this would have to be written as:
 *
 *    <pre>
 * macro pager \cx '<call>,r13on<enter>'</pre>
 *
 *    Another example not working any longer:
 *
 *    <pre>
 * macro index I "c!\n"</pre>
 *
 *    since the event handler doesn't know what to do with @c c and that
 *    @c \n means "finish input." Correct will be:
 *
 *    <pre>
 * macro index I '<change-folder>!<enter></pre>
 *
 *    The same counts for all magic keys which must be written as their
 *    function names.
 *
 *    When scanning such sequences, function names will be converted
 *    into events to trigger. After the function name, we'll look for
 *    more input until we find <code><enter></code>. The registered
 *    handler will get everything we think is input as input. The input
 *    in:
 *
 *    <pre>
 * macro index I '<change-folder>!<exit>'</pre>
 *
 *    is just <code>!</code> since <code><exit></code> is a function. In
 *    this case, the input is unfinished which the handler will be told.
 *    On the contrary, the following is complete input and runs without
 *    interaction:
 *
 *    <pre>
 * macro index I '<change-folder>!<enter><exit></pre>
 *
 *    As a consequence, any input not terminated with <code><enter></code>
 *    will cause a break in processing (well, the UI may decide to
 *    ignore waiting for input which we cannot know due to separation.)
 *
 *    <code>An important consequence of not being abled to process raw
 *    keystrokes any longer is that @c bind and @c macro have no
 *    semantical difference any longer.</code>
 */
/* }}} */
/**
 * @file muttng/event/event.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Event handling interface
 */
#ifndef MUTTNG_EVENT_H
#define MUTTNG_EVENT_H

#include "core/buffer.h"

#include "libmuttng/debug.h"

/**
 * Core event handler class.
 */
class Event {
  public:
    /** constructor */
    Event (Debug* debug);
    /** destructor */
    ~Event (void);
    /** init, ie build up all tables */
    bool init (void);

    /** Valid contexts, ie menus or screens */
    enum context {
      /**
       * Generic. These will be always tried first. */
      C_GENERIC = 0,
      /** Index screen. */
      C_INDEX,
      /** Pager screen. */
      C_PAGER,
      /** Help screen. */
      C_HELP,
      /** For static array sizes. */
      C_LAST
    };

    /** Valid events, ie the former @c OP_* and more */
    enum event {
      /** Triggered when entering any menu */
      E_CONTEXT_ENTER = 0,
      /** Triggered when leaving any menu */
      E_CONTEXT_LEAVE,
      /** For static array sizes. */
      E_LAST
    };

    /** Return state of handlers */
    enum state {
      /** everything went fine. */
      S_OK = 0,
      /** the command failed. */
      S_FAIL,
      /** the command failed due to invalid input. */
      S_INPUT
    };

    /**
     * Bind an internal function to an event.
     * This is to be used to bind UI functions to events and
     * bind any core logic handlers to events. To separate between UI
     * and core logic, this doesn't involve key sequences.
     * @param context Context to bind to.
     * @param event Specific event within context to bind to.
     * @param input Whether handler expects user input.
     * @param handler Handler callback function.
     * @sa eventhandler_t.
     */
    void bindInternal (Event::context context,
                       Event::event event,
                       bool input,
                       Event::state (*handler) (Event::context context,
                                                Event::event event,
                                                const char* input,
                                                bool complete,
                                                unsigned long data));

    /**
     * Bind a "user" function to an event.
     * To separate between core logic and UI, this maps a key sequence
     * to an event within a context.
     * @param context Context to bind to.
     * @param key Key sequence.
     * @param func Function string. These are the arguments from
     *             commands like @c bind and @c macro.
     * @param error Error buffer. As the input will be checked for
     *              validity, the error message is put into the
     *              given error buffer.
     * @return Whether operation succeeded.
     */
    bool bindUser (Event::context context, const char* key,
                   const char* func, buffer_t* error);

    /**
     * Internally set the current to a different context.
     * This will trigger the Event::E_CONTEXT_ENTER event.
     * @param context New context.
     * @param data Any data passed to event handler.
     */
    void setContext (Event::context, unsigned long data);

    /**
     * Internally unset the current to an undefined context.
     * This will trigger the Event::E_CONTEXT_LEAVE event.
     * @param data Abitrary data passed to handler.
     */
    void unsetContext (unsigned long data);

    /**
     * Enqueue an event.
     * @param event Event to put into queue.
     * @param data Any data passed to the event handler.
     */
    void enqueue (Event::event event, unsigned long data);

  private:
    /** Context stack. */
    Event::context currentContext;
    /** debug */
    Debug* debug;
};

/**
 * Typedef of an event handler's signature for better readability.
 * @param context Current context the event occured in.
 * @param event The actual event.
 * @param input Any user input. If none given but the handler expects
 *              input, it has to get it and wait. This is required to
 *              implement the @c push command for all interfaces. Given
 *              we really add a GUI and it needs to query some input via
 *              a popup dialog, there's no portable way to push some
 *              input given in a macro into the dialog. Thus, we parse
 *              the input already and hand it over.
 * @param complete Whether the input is complete. Within macros, users
 *                 may want to have unbuffered input, i.e. by intention
 *                 do something like <code>push s=some.folder</code> to
 *                 let muttng prompt and wait.
 * @param data Arbitrary data passed through Event::enqueue().
 * @return State after execution.
 */
typedef Event::state eventhandler_t (Event::context context,
                                     Event::event event,
                                     const char* input,
                                     bool complete,
                                     unsigned long data);

#endif /* !MUTTNG_EVENT_H */

/** @} */
