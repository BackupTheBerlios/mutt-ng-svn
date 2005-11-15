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
 *        and emit events.
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
 *   request to emit them. We'll perform event lookup and emit them.
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
 * macro pager \\cx ,r13on</pre>
 *
 *    (which is the entry point for a toggle-rot13-macro) because we can't
 *    make a GUI believe it got the key sequence <code>,r13on</code>
 *    which itself is bound to a macro. This is a simple example with only one key
 *    sequence, more complicated examples are possible of course. With
 *    the new type of event handling, this would have to be written as:
 *
 *    <pre>
 * macro pager \\cx '&lt;call&gt;,r13on&lt;enter&gt;'</pre>
 *
 *    Another example not working any longer:
 *
 *    <pre>
 * macro index I "c!\n"</pre>
 *
 *    since the event handler doesn't know what to do with @c c and that
 *    @c \\n means "finish input." Correct will be:
 *
 *    <pre>
 * macro index I '<change-folder>!&lt;enter&gt;</pre>
 *
 *    The same counts for all magic keys which must be written as their
 *    function names.
 *
 *    When scanning such sequences, function names will be converted
 *    into events to trigger. After the function name, we'll look for
 *    more input until we find <code>&lt;enter&gt;</code>. The registered
 *    handler will get everything we think is input as input. The input
 *    in:
 *
 *    <pre>
 * macro index I '<change-folder>!&lt;exit&gt;'</pre>
 *
 *    is just <code>!</code> since <code>&lt;exit&gt;</code> is a function. In
 *    this case, the input is unfinished which the handler will be told.
 *    On the contrary, the following is complete input and runs without
 *    interaction:
 *
 *    <pre>
 * macro index I '<change-folder>!&lt;enter&gt;&lt;exit&gt;</pre>
 *
 *    As a consequence, any input not terminated with <code>&lt;enter&gt;</code>
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
 * @brief Interface: Event handling
 */
#ifndef MUTTNG_EVENT_H
#define MUTTNG_EVENT_H

#include "core/buffer.h"
#include "core/list.h"

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
#include "contextlist.h"
      /** For static array sizes. */
      C_LAST
    };

    /** Valid events */
    enum event {
#include "eventlist.h"
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
     * @param self Pointer to this. All callbacks must be static so
     *             we need a pointer to the object for dynamic member
     *             access.
     * @param handler Handler callback function.
     * @sa eventhandler_t.
     */
    void bindInternal (Event::context context,
                       Event::event event,
                       bool input, void* self,
                       Event::state (*handler) (Event::context context,
                                                Event::event event,
                                                const char* input,
                                                bool complete,
                                                void* self,
                                                unsigned long data));

    /**
     * Emit an event and deliver to all registered handlers.
     * @param file Source file calling us (for debugging).
     * @param line Line in source file (for debugging).
     * @param context Context of event
     * @param event Event to emit.
     * @param input User input for handler.
     * @param complete Whether input for handler is complete or
     *                 prompting is to be done by handler.
     * @param data Arbitrary data passed through.
     */
    bool _emit (const char* file, int line, Event::context context,
                Event::event event, const char* input, bool complete,
                unsigned long data);

    /**
     * Macro to get occurance of call into _emit().
     * @param C Context.
     * @param E Event.
     * @param I Input.
     * @param F Complete.
     * @param D Data.
     */
#define emit(C,E,I,F,D) _emit(__FILE__,__LINE__,C,E,I,F,D)

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
     * @param file Source file calling us (for debugging).
     * @param line Line in source file (for debugging).
     * @param context New context.
     * @param data Any data passed to event handler.
     */
    void _setContext (const char* file, int line, Event::context context,
                      unsigned long data);

    /**
     * Macro to get occurance of call into _setContext().
     * @param C Context.
     * @param D Data.
     */
#define setContext(C,D) _setContext(__FILE__,__LINE__,C,D)

    /**
     * Internally unset the current to an undefined context.
     * This will trigger the Event::E_CONTEXT_LEAVE event
     * after leaving it and trigger Event::E_CONTEXT_REENTER for
     * the new topmost context on our stack.
     * @param file Source file calling us (for debugging).
     * @param line Line in source file (for debugging).
     * @param data Abitrary data passed to handler.
     */
    void _unsetContext (const char* file, int line, unsigned long data);

    /**
     * Macro to get occurance of call into _unsetContext().
     * @param D Data.
     */
#define unsetContext(D) _unsetContext(__FILE__,__LINE__,D)

    /**
     * Get event for a given key of current context.
     * This is to be used for a GUI building up its menu shortcuts.
     * A GUI likely wants to disable event handler to not trigger enter
     * or leave events as it needs to switch contexts for
     * initialization.
     * @param event Event
     * @return Key.
     * @sa disable(), enable().
     */
    const char* getKey (Event::event event);

    /**
     * Get event for a given key of current context.
     * This is to be used for curses' @c getch() to get event to
     * enqeue().
     * @param key Key
     * @return Event
     */
    Event::event getEvent (const char* key);

    /** Temporarily disable event handler. */
    void disable (void);

    /** Enable again after disabling */
    void enable (void);

    /**
     * Get textual name of a context.
     * @param context Context.
     * @return Name.
     */
    static const char* getContextName (Event::context context);

    /**
     * Get textual name of event.
     * @param event Event.
     * @return Name.
     */
    static const char* getEventName (Event::event event);

  private:
    /** Context stack. */
    list_t* contextStack;
    /** debug */
    Debug* debug;
    /** active? */
    bool active;
    /** big table of all event handlers */
    list_t* handlers[C_LAST][E_LAST];
    /** big table of all keys for events */
    char* keys[C_LAST][E_LAST];
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
 * @param self Pointer to this. All callbacks must be static so
 *             we need a pointer to the object for dynamic member access.
 * @param data Arbitrary data passed through.
 * @return State after execution.
 */
typedef Event::state eventhandler_t (Event::context context,
                                     Event::event event,
                                     const char* input,
                                     bool complete,
                                     void* self,
                                     unsigned long data);

#endif /* !MUTTNG_EVENT_H */

/** @} */
