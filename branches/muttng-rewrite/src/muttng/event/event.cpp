/* vim:foldmethod=marker:foldlevel=0:ft=cpp
 */
/* doxygen documentation {{{ */
/**
 * @ingroup muttng
 * @addtogroup muttng_event Event Handling
 * @{
 *
 *   @section muttng_event_intro Introduction
 *
 *     Event handling is one of the most or even the most important part
 *     of the UI built on top of libmuttng since many many things are
 *     now done with simple event handling:
 *
 *       - implementation of all hooks
 *       - user interface notification upon configuration changes
 *       - screen switches
 *       - all function handling and binding
 *       - all @c bind and @c macro implementations
 *       - ...
 *
 *     The implementation is based on the following two concepts:
 *
 *       -# There are so called "contexts", that is a semantic grouping
 *          of events. Currently these are the former screens/menus, but
 *          other contexts may follow. Contexts are remembered using a
 *          stack to centralize the context switching within the event
 *          handler.
 *       -# There are events, that is simply a numeric ID to which any
 *          handler within the application can be bound (since an event
 *          handler is a property of Muttng and all application classes
 *          must be derived from it, all classes have access to the
 *          event handler.) Any piece of
 *          code can demand the central event handler to emit a specific
 *          event which will then notify all handlers. This way the
 *          logic and many parts are really de-coupled.
 *
 *   @section muttng_event_context Context (stack)
 *
 *     The stack needs to be organized as a stack to de-couple different
 *     parts. As certain parts may want to perform some action upon a
 *     context switch, there must be an event for it. However, when only
 *     knowing about the current context, the part triggering the switch
 *     would have to save the old context to emit a re-entering of the
 *     old context after leaving the new one. To centralize this within
 *     the event handler itself, there's a stack. The current context
 *     can be set and unset with Event::setContext() and
 *     Event::unsetContext() and the event handler will emit the
 *     following events on its own:
 *
 *       -# Event::setContext() will emit Event::E_CONTEXT_ENTER and
 *          push the new context on the stack.
 *       -# Event::unsetContext() will first emit Event::E_CONTEXT_LEAVE,
 *          of course, and pop the context we're leaving. If afterwards
 *          the stack is not empty, it will emit
 *          Event::E_CONTEXT_REENTER.
 *
 *     This approach is useful in implementing a <code>context-hook</code>
 *     triggered upon Event::E_CONTEXT_ENTER and
 *     Event::E_CONTEXT_REENTER. This can be used, for example, like
 *     this:
 *
 *     <pre>
 * context-hook . '&lt;enter-command&gt;set sidebar_width=20&lt;enter&gt;'
 * context-hook help '&lt;enter-command&gt;unset sidebar_width&lt;enter&gt;'</pre>
 *
 *     to turn off the sidebar when viewing the help screen.
 *
 *     The Event::C_GENERIC is special, since even if there's no handler
 *     for an event of the current context, the event for the generic
 *     context will always be tried.
 *
 *   @section muttng_event_event Events
 *
 *     There're basically only two types of events:
 *
 *       -# Internal events. These include the context switches, a
 *          configuration change, etc. In fact this is everything except
 *          the second group.
 *       -# User events. These are the former functions.
 *
 *   @section muttng_event_impl Implementation
 *
 *     @subsection muttng_event_impl_ov Overhead
 *
 *       To reduce overhead as far as possible, we take the big table
 *       approach: Event::handlers is a multi-dimensional table mapping
 *       an event of a context to a list of handlers to be called. This
 *       ensures we have O(1) access to all handlers. Also the table
 *       size is constant except for the list of handlers, of course.
 *       The drawback is that certrain events aren't valid in all
 *       contexts so that this wastes memory.
 *
 *     @subsection muttng_event_impl_bind Binding
 *
 *       As the lookup for the list works in O(1), there's not really
 *       much overhead for binding.
 *
 *       Unfortunately, we cannot register dynamic (i.e. non-static)
 *       members of an object as the callback handler. The solution
 *       currently is pretty dangrous: when using Event::bindInternal(),
 *       the caller must provide a self pointer of type
 *       <code>void*</code>. This is the pointer to <code>this</code>
 *       casted to <code>void*</code>. The handler must cast it back to
 *       a pointer of the object type it is part of. This way, the
 *       handler has access to non-static members. This can be pretty
 *       dangerous but enables us to have all tables constant by means
 *       of size.
 *
 *       Pretty dangerous, too, is the way data is passed. A handler can
 *       only be used to catch one type event since the data passed to
 *       handlers depends on the actual event. Everything is casted to
 *       <code>unsigned long</code>. Of course, the handler gets told
 *       for which event of which context it was called so it could do
 *       distinction when casting data back to its original form...
 *
 *       That the whole binding isn't really typesafe but requires lots
 *       of care is something which needs to be improved. At best, we
 *       keep O(1) access and even reduce memory waste.
 *
 *       The type of data passed on events is:
 *
 *         - Event::E_CONTEXT_ENTER, Event::E_CONTEXT_LEAVE,
 *           Event::E_CONTEXT_REENTER: <code>0</code>
 *         - Event::E_OPTION_CHANGE: <code>option_t*</code>
 *         - Event::E_HELP: <code>0</code>
 *
 *     @subsection muttng_event_bind_hacking Hacking
 *
 *       As the order of elements stored in tables matters, the source
 *       being compiled is created automatically by the
 *       <code>event.pl</code> script in the <code>muttng/event/</code>
 *       directory. It reads the files
 *       <code>muttng/event/CONTEXTS</code> and
 *       <code>muttng/event/EVENTS</code> and replaces magic words in
 *       <code>event.cpp.in</code> and <code>event.h.in</code>
 *       respectively. Doxygen documentation is created, too.
 *
 *       <b>When making changes to the implementation, do them in the
 *       latter two files only!</b>
 *
 *       The magic words are (with one more trailing and one more
 *       leading underscore [replacement is done via regex so we
 *       we would get in trouble here...]):
 *
 *         -# @c _CONTEXT_ENUM_ builds up Event::context
 *         -# @c _EVENT_ENUM_ builds up Event::event
 *         -# @c _CONTEXT__DESCR_ builds up CtxStr
 *         -# @c _EVENT_DESCR_ builds up EvStr
 *         -# @c _EVENT_VALID_ builds up EvValid
 *
 *       <code>CONTEXTS</code> and <code>EVENTS</code> contain simple
 *       space-separated fields.
 *
 *       The fields of <code>CONTEXTS</code> have the following meaning:
 *
 *         -# context ID for Event::context (i.e. <code>C_GENERIC</code>)
 *         -# context description for the @c bind and @c macro features
 *
 *       The fields of <code>EVENTS</code> have the followin meaning:
 *
 *         -# @c OR'ed context ID for which the event is valid to be
 *            store in EvValid. There must be no space between |.
 *            Example: <code>C_INDEX|C_PAGER|...</code> for
 *            <code>&lt;help&gt;</code> which is valid in all screens
 *            except help due to looping.
 *         -# event ID for Event::event (i.e. <code>E_HELP</code>)
 *         -# the function name for the event the user can bind to or
 *            @c NULL if none, i.e. the event is internal.
 *         -# event category for functions and @c NULL otherwise. This
 *            can be used by the UI to group functions in the help
 *            screen by category and key rather than only by key.
 *         -# the default key binding for functions and @c NULL
 *            otherwise.
 *         -# the description of a function for the help screen and
 *            manual and @c NULL for internal events.
 *
 *       This format currently implies that a function must always have
 *       the same default keybinding for all contexts it's valid in.
 */
/* }}} */
/**
 * @file muttng/event/event.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Event handling
 */
#include "core/mem.h"
#include "core/str.h"

#include "event.h"
#include "../muttng.h"

/** how we internally register a handler */
typedef struct {
  /** the handler itself */
  eventhandler_t* handler;
  /** casted pointer of objects 'this' to be passed back */
  void* self;
  /** whether handler expects input */
  bool input;
} handle_t;

/**
 * For @c OR'ing the valid contexts for an event, we use some bit
 * shifting and the values of Event::context as bit numbers. This macro
 * is a shorthand for it.
 * @param X Event::context.
 */
#define CTX(X)          (1<<X)

/* ctx2str table {{{ */

/** Table mapping Event::context to string */
static const char* CtxStr[Event::C_LAST+1] = {
  /** context string for C_GENERIC */
  "generic",
  /** context string for C_INDEX */
  "index",
  /** context string for C_PAGER */
  "pager",
  /** context string for C_HELP */
  "help",
  /** context string for C_LAST */
  NULL
};

/* }}} */
/* event2str table {{{ */

/** Table mapping events to string */
static const char* EvStr[Event::E_LAST+1] = {
  /** event string for E_CONTEXT_ENTER */
  "E_CONTEXT_ENTER",
  /** event string for E_CONTEXT_LEAVE */
  "E_CONTEXT_LEAVE",
  /** event string for E_CONTEXT_REENTER */
  "E_CONTEXT_REENTER",
  /** event string for E_OPTION_CHANGE */
  "E_OPTION_CHANGE",
  /** event string for E_HELP */
  "E_HELP",
  /** event string for E_LAST */
  NULL
};

/* }}} */
/* event validity table {{{ */

/** Table telling us for which context an event is valid */
static const int EvValid[Event::E_LAST+1] = {
  /** for which contexts E_CONTEXT_ENTER is valid */
  CTX(Event::C_GENERIC),
  /** for which contexts E_CONTEXT_LEAVE is valid */
  CTX(Event::C_GENERIC),
  /** for which contexts E_CONTEXT_REENTER is valid */
  CTX(Event::C_GENERIC),
  /** for which contexts E_OPTION_CHANGE is valid */
  CTX(Event::C_GENERIC),
  /** for which contexts E_HELP is valid */
  CTX(Event::C_INDEX) | CTX(Event::C_PAGER),
  /** for which contexts E_LAST is valid */
  0
};

/* }}} */

Event::Event (Debug* debug) {
  int i = 0, j = 0;

  this->debug = debug;
  this->active = true;
  this->contextStack = NULL;

  for (i = 0; i < C_LAST; i++)
    for (j = 0; j < E_LAST; j++) {
      handlers[i][j] = NULL;
      keys[i][j] = NULL;
    }
}

Event::~Event (void) {
  int i = 0, j = 0;
  handle_t* handler = NULL;

  DEBUGPRINT(1,("cleanup event handler"));

  while (!(list_empty (contextStack)))
    unsetContext (0);

  /*
   * when doing cleanup, loop over all handlers and print
   * a big fat warning if some handler is still bound; with the
   * void* cast of 'this' and friends all of this is pretty
   * dangerous so be strict here
   */
  for (i = 0; i < C_LAST; i++)
    for (j = 0; j < E_LAST; j++) {
      if (!list_empty (handlers[i][j])) {
        while (!list_empty (handlers[i][j])) {
          handler = (handle_t*) list_pop_front (&handlers[i][j]);
          DEBUGPRINT(D_EVENT,("+++ WARNING +++ callback 0x%x of 0x%x still "
                        "bound for ctx=%s ev=%s", handler->handler,
                        handler->self, NONULL (CtxStr[i]), NONULL (EvStr[j])));
          mem_free (&handler);
        }
      }
    }
  list_del (&contextStack, NULL);
}

bool Event::init (void) {
  DEBUGPRINT(1,("init event handler"));
  setContext (C_GENERIC, 0);
  return (true);
}

/**
 * @bug Maybe we need to check for duplicates here.
 */
void Event::bindInternal (Event::context context,
                          Event::event event,
                          bool input, void* self,
                          Event::state (*handler) (Event::context context,
                                                   Event::event event,
                                                   const char* input,
                                                   bool complete, void* self,
                                                   unsigned long data)) {
  handle_t* handle = (handle_t*) mem_malloc (sizeof (handle_t));
  handle->handler = handler;
  handle->self = self;
  handle->input = input;
  list_push_back (&handlers[context][event], (LIST_ITEMTYPE) handle);
  DEBUGPRINT(D_EVENT,("bound callback 0x%x of 0x%x to ctx=%s ev=%s",
                handler, self, NONULL (CtxStr[context]),
                NONULL (EvStr[event])));
}

void Event::unbindInternal (void* self) {
  int i = 0, j = 0;
  for (i = 0; i < C_LAST; i++)
    for (j = 0; j < E_LAST; j++)
      unbindInternal ((Event::context) i, (Event::event) j, self);
}

/**
 * @bug Maybe we need to check for duplicates here.
 */
void Event::unbindInternal (Event::context context, Event::event event,
                            void* self) {
  list_t** list = NULL;
  handle_t* handler = NULL;
  int i = 0;

  if (!(list = &handlers[context][event]))
    return;
  for (i = 0; !list_empty((*list)) && i < (int) (*list)->length; i++) {
    handler = (handle_t*) (*list)->data[i];
    if (handler->self == self) {
      DEBUGPRINT(D_EVENT,("unbound callback 0x%x of 0x%x from ctx=%s ev=%s",
                    handler->handler, self, NONULL (CtxStr[context]),
                    NONULL (EvStr[event])));
      handler = (handle_t*) list_pop_idx (list, i);
      mem_free (&handler);
      i--;
    }
  }
  if (!list_empty ((*list)))
    mem_free (list);
}

bool Event::_emit (const char* file, int line, Event::context context,
                   Event::event event, const char* input, bool complete,
                   unsigned long data) {
  list_t* list = NULL;
  int i = 0;
  handle_t* handle = NULL;
  Event::state state = S_OK;

  DEBUGPRINT(D_EVENT,("emit: ctx=%s, ev=%s, in='%s', compl=%d, d=0x%x from %s:%d",
                CtxStr[context], EvStr[event], NONULL (input), complete,
                data, NONULL (file), line));

  if (!active) {
    DEBUGPRINT(D_EVENT,("event handler disabled, skipping"));
    return (true);
  }

  if (!(list = this->handlers[context][event]) &&
      !(list = this->handlers[Event::C_GENERIC][event])) {
    DEBUGPRINT(D_EVENT,("no handlers found, skipping"));
    return (true);
  }

  if (!(EvValid[event] & CTX(context))) {
    DEBUGPRINT(D_EVENT,("event not allowed for context"));
    return (false);
  }

  for (i = 0; i < list->length; i++) {
    handle = (handle_t*) list->data[i];
    if (handle->input && !input) {
      /* error */
    }
    if ((state = handle->handler (context, event, input, complete,
                                  handle->self, data)) != S_OK) {
      /* error */
      return (false);
    }
  }
  return (true);
}

bool Event::bindUser (Event::context context, const char* key,
                      const char* func, buffer_t* error) {
  (void) context;
  (void) key;
  (void) func;
  (void) error;
  return (true);
}

void Event::_setContext (const char* file, int line,
                        Event::context context, unsigned long data) {
  DEBUGPRINT(D_EVENT,("enter ctx=%s from %s:%d", CtxStr[context], NONULL (file), line));
  emit (context, E_CONTEXT_ENTER, NULL, false, data);
  list_push_back (&contextStack, (LIST_ITEMTYPE) context);
}

void Event::_unsetContext (const char* file, int line, unsigned long data) {
  Event::context context;

  context = (Event::context) list_pop_back (&contextStack);
  DEBUGPRINT(D_EVENT,("leave ctx=%s from %s:%d", CtxStr[context], NONULL (file), line));
  emit (context, E_CONTEXT_LEAVE, NULL, false, data);
  if (!list_empty (contextStack)) {
    context = (Event::context) contextStack->data[contextStack->length-1];
    DEBUGPRINT(D_EVENT,("re-enter ctx=%s from %s:%d", CtxStr[context],
                  NONULL (file), line));
    emit (context, E_CONTEXT_REENTER, NULL, false, data);
  }
}

const char* Event::getKey (Event::event event) {
  (void) event;
  return (NULL);
}

Event::event Event::getEvent (const char* key) {
  (void) key;
  return (E_LAST);
}

void Event::disable (void) {
  active = false;
  DEBUGPRINT(D_EVENT,("event handler disabled"));
}

void Event::enable (void) {
  DEBUGPRINT(D_EVENT,("event handler enabled"));
  active = true;
}

const char* Event::getContextName (Event::context context) {
  return (CtxStr[context]);
}

const char* Event::getEventName (Event::event event) {
  return (EvStr[event]);
}
