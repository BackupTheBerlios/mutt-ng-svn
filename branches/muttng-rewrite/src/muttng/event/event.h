/** @ingroup muttng_event */
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
      /** context ID for "generic" */
      C_GENERIC = 0,
      /** context ID for "index" */
      C_INDEX,
      /** context ID for "pager" */
      C_PAGER,
      /** context ID for "help" */
      C_HELP,
      /** For static array sizes. */
      C_LAST
    };

    /** Valid events */
    enum event {
      /** event ID for <code>&lt;NULL&gt;</code> */
      E_CONTEXT_ENTER = 0,
      /** event ID for <code>&lt;NULL&gt;</code> */
      E_CONTEXT_LEAVE,
      /** event ID for <code>&lt;NULL&gt;</code> */
      E_CONTEXT_REENTER,
      /** event ID for <code>&lt;NULL&gt;</code> */
      E_OPTION_CHANGE,
      /** event ID for <code>&lt;help&gt;</code> */
      E_HELP,
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
     * Unbind all internal event handlers for an object.
     * @param self Object's @c this casted to <b>void*</b>.
     */
    void unbindInternal (void* self);

    /**
     * Unbind handler for specific event and context.
     * @param context Context
     * @param event Event
     * @param self Object's @c this casted to @c void*.
     */
    void unbindInternal (Event::context context, Event::event event,
                         void* self);

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
