/** @ingroup muttng_event */
/**
 * @file muttng/event/event.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Event handling
 */
#ifndef MUTTNG_EVENT_H
#define MUTTNG_EVENT_H

#include <vector>

#include "core/buffer.h"
#include "core/list.h"

#include "libmuttng/debug.h"
#include "libmuttng/signal.h"
#include "libmuttng/util/url.h"

/**
 * Info about an event bound.
 * Returned by Event::getHelp()
 */
typedef struct {
  /** key (sequence) the event is bound to */
  char* key;
  /** default key */
  const char* defkey;
  /** function name */
  const char* name;
  /** translated help string */
  const char* help;
} binding_t;

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

    /** Valid groups */
    enum group {
      /** group ID for "Internal" */
      G_INTERNAL = 0,
      /** group ID for "Generic" */
      G_GENERIC,
      /** group ID for "Reply/Forward" */
      G_REPLY,
      /** group ID for "Movement" */
      G_MOVE,
      /** group ID for "Editing" */
      G_EDIT,
      /** group ID for "Threading" */
      G_THREAD,
      /** For static array sizes. */
      G_LAST
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
      /** event ID for <code>&lt;show-version&gt;</code> */
      E_SHOW_VERSION,
      /** event ID for <code>&lt;reply&gt;</code> */
      E_REPLY,
      /** event ID for <code>&lt;list-reply&gt;</code> */
      E_LIST_REPLY,
      /** event ID for <code>&lt;group-reply&gt;</code> */
      E_GROUP_REPLY,
      /** event ID for <code>&lt;page-up&gt;</code> */
      E_PAGE_UP,
      /** event ID for <code>&lt;edit-message&gt;</code> */
      E_EDIT_MESSAGE,
      /** event ID for <code>&lt;break-thread&gt;</code> */
      E_BREAK_THREAD,
      /** event ID for <code>&lt;link-thread&gt;</code> */
      E_LINK_THREAD,
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
     */
    void _setContext (const char* file, int line, Event::context context);

    /**
     * Macro to get occurance of call into _setContext().
     * @param C Context.
     */
#define setContext(C) _setContext(__FILE__,__LINE__,C)

    /**
     * Internally unset the current to an undefined context.
     * This will trigger the Event::E_CONTEXT_LEAVE event
     * after leaving it and trigger Event::E_CONTEXT_REENTER for
     * the new topmost context on our stack.
     * @param file Source file calling us (for debugging).
     * @param line Line in source file (for debugging).
     */
    void _unsetContext (const char* file, int line);

    /**
     * Macro to get occurance of call into _unsetContext().
     */
#define unsetContext(D) _unsetContext(__FILE__,__LINE__)

    /**
     * Get event for a given key of current context.
     * This is to be used for a GUI building up its menu shortcuts.
     * A GUI likely wants to disable event handler to not trigger enter
     * or leave events as it needs to switch contexts for
     * initialization.
     * @param event Event
     * @return Key.
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

    /**
     * Get current context.
     * @return Event.
     */
    Event::context getContext (void);

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

    /**
     * Get textual name of group.
     * @param group Event.
     * @return Name.
     */
    static const char* getGroupName (Event::group group);

    /**
     * See whether an event is allowed within a context.
     * @param context Context.
     * @param event Event.
     * @return Yes/No.
     */
    bool eventValid (Event::context context, Event::event event);

    /**
     * For a given context, get all groups within.
     * @param context Context.
     * @return Vector of groups or @c NULL.
     */
    std::vector<Event::group>* getGroups (Event::context context);

    /**
     * For a context and a group, get all valid events.
     * @param context Context.
     * @param group Group.
     * @return Vector of events or @c NULL.
     */
    std::vector<Event::event>* getEvents (Event::context context,
                                          Event::group group);

    /**
     * For a context, get all valid events.
     * @param context Context.
     * @return Vector of events or @c NULL.
     */
    std::vector<Event::event>* getEvents (Event::context context);

    /**
     * For a given context and event, get binding_t* info for help
     * screens.
     * @param context Context.
     * @param event Event.
     * @return Binding.
     */
    binding_t* getHelp (Event::context context, Event::event event);

    /** signal emitted when context changes */
    Signal2<Event::context,Event::event> sigContextChange;
    /** signal emitted when numeric config option changes */
    Signal3<Event::context,const char*,int> sigNumOptChange;
    /** signal emitted when url config option changes */
    Signal3<Event::context,const char*,url_t*> sigURLOptChange;

  private:
    /** Context stack. */
    std::vector<Event::context>* contextStack;
    /** debug */
    Debug* debug;
    /** big table of all keys for events */
    binding_t bindings[C_LAST][E_LAST];
};

/**
 * Postfix @c ++ operator for Event::context and other enums.
 */
template<typename T>
inline T operator++ (T& t, int value) { (void) value; return t = (T)(t+1); }

/** first event */
#define E_0     ((Event::event) 0)
/** first context */
#define C_0     ((Event::context) 0)
/** first group */
#define G_0     ((Event::group) 0)

#endif /* !MUTTNG_EVENT_H */
