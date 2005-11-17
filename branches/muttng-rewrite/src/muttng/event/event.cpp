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
 *     The implementation is based on the following concepts:
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
 *       -# In addition to these, for user interfaces, events are also
 *          sorted into groups. For example, a user interface could want
 *          to sort functions by semantic instead of only by assigned
 *          keys.
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
 *       As the lookup for the arrays works in O(1), there's not really
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
 *       <code>muttng/event/CONTEXTS</code>,
 *       <code>muttng/event/EVENTS</code> and 
 *       <code>muttng/event/GROUPS</code> and replaces magic words in
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
 *         -# @c _GROUP_ENUM_ builds up Event::group
 *         -# @c _CONTEXT__DESCR_ builds up CtxStr
 *         -# @c _EVENT_DESCR_ builds up EvStr
 *         -# @c _GROUP_DESCR_ builds up GroupStr
 *         -# @c _EVENT_VALID_ builds up EvValid
 *         -# @c _GROUP_VALID_ builds up GroupValid
 *         -# @c _EVENT_HELP_ builds up EvHelp
 *       
 *       In addition, @c _BIND_ produces the code to setup the default
 *       bindings for functions, i.e. the bindings array.
 *
 *       <code>CONTEXTS</code>, <code>muttng/event/GROUPS</code> and
 *       <code>EVENTS</code> contain simple space-separated fields.
 *
 *       The fields of <code>CONTEXTS</code> have the following meaning:
 *
 *         -# context ID for Event::context (i.e. <code>C_GENERIC</code>)
 *         -# context description for the @c bind and @c macro features
 *
 *       Similarily, the fileds of <code>GROUPS</code> have the
 *       following meaning:
 *
 *         -# group ID for Event::group (i.e. <code>G_INTERNAL</code>)
 *         -# group description the fancy help screens
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
 *         -# the group ID of the event (use @c G_INTERNAL for all
 *            non-functions)
 *         -# the default key binding for functions and @c NULL
 *            otherwise.
 *         -# the description of a function for the help screen and
 *            manual and @c NULL for internal events.
 *
 *       This format currently implies that a function must always have
 *       the same default keybinding for all contexts it's valid in.
 *
 *     @subsection muttng_event_bind_usage Using the event handler
 *
 *       It can be used in the following ways:
 *
 *         -# bind to and thus catch any event
 *         -# emit any event
 *         -# query for listings of events
 *
 *       The first is achieved via the @c bind* functions.
 *
 *       The second is achieved via the emit() function.
 *
 *       For the third, here are some usage scenarios:
 *
 *         - a full listing (including everything application-interal,
 *           i.e. non-functions) can be obtained via getHelp() and
 *           looping over Event::context and Event::event
 *         - @c muttng-conf(1) uses filters available: it retrieves a
 *           list of valid groups for every context. While ignoring
 *           @c G_INTERNAL, it retrieves the list of all known events
 *           for all groups it got. For every event, it then uses
 *           getHelp() to get data to display.
 */
/* }}} */
/**
 * @file muttng/event/event.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Event handling
 */
#include "core/mem.h"
#include "core/str.h"
#include "core/intl.h"

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
  "help",
  /** event string for E_SHOW_VERSION */
  "show-version",
  /** event string for E_REPLY */
  "reply",
  /** event string for E_LIST_REPLY */
  "list-reply",
  /** event string for E_GROUP_REPLY */
  "group-reply",
  /** event string for E_PAGE_UP */
  "page-up",
  /** event string for E_EDIT_MESSAGE */
  "edit-message",
  /** event string for E_BREAK_THREAD */
  "break-thread",
  /** event string for E_LINK_THREAD */
  "link-thread",
  /** event string for E_LAST */
  NULL
};

/* }}} */
/* event2help table {{{ */

/** Table mapping events to string */
static const char* EvHelp[Event::E_LAST+1] = {
  /** help string for E_CONTEXT_ENTER */
  "E_CONTEXT_ENTER",
  /** help string for E_CONTEXT_LEAVE */
  "E_CONTEXT_LEAVE",
  /** help string for E_CONTEXT_REENTER */
  "E_CONTEXT_REENTER",
  /** help string for E_OPTION_CHANGE */
  "E_OPTION_CHANGE",
  /** help string for E_HELP */
  N_("Show help screen"),
  /** help string for E_SHOW_VERSION */
  N_("Show version information"),
  /** help string for E_REPLY */
  N_("Reply to current message"),
  /** help string for E_LIST_REPLY */
  N_("List-reply to current message"),
  /** help string for E_GROUP_REPLY */
  N_("Group-reply to current message"),
  /** help string for E_PAGE_UP */
  N_("Page up"),
  /** help string for E_EDIT_MESSAGE */
  N_("Edit current message"),
  /** help string for E_BREAK_THREAD */
  N_("Break thread at current message"),
  /** help string for E_LINK_THREAD */
  N_("Link tagged to current message in thread"),
  /** help string for E_LAST */
  NULL
};

/* }}} */
/* group2str table {{{ */

/** Table mapping events to string */
static const char* GroupStr[Event::G_LAST+1] = {
  /** group string for G_INTERNAL */
  "Internal",
  /** group string for G_GENERIC */
  "Misc.",
  /** group string for G_MOVE */
  "Movement",
  /** group string for G_REPLY */
  "Reply/Forward",
  /** group string for G_EDIT */
  "Editing",
  /** group string for G_LAST */
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
  /** for which contexts E_SHOW_VERSION is valid */
  CTX(Event::C_GENERIC),
  /** for which contexts E_REPLY is valid */
  CTX(Event::C_INDEX) | CTX(Event::C_PAGER),
  /** for which contexts E_LIST_REPLY is valid */
  CTX(Event::C_INDEX) | CTX(Event::C_PAGER),
  /** for which contexts E_GROUP_REPLY is valid */
  CTX(Event::C_INDEX) | CTX(Event::C_PAGER),
  /** for which contexts E_PAGE_UP is valid */
  CTX(Event::C_GENERIC),
  /** for which contexts E_EDIT_MESSAGE is valid */
  CTX(Event::C_INDEX) | CTX(Event::C_PAGER),
  /** for which contexts E_BREAK_THREAD is valid */
  CTX(Event::C_INDEX),
  /** for which contexts E_LINK_THREAD is valid */
  CTX(Event::C_INDEX),
  /** for which contexts E_LAST is valid */
  0
};
/* }}} */
/* group validity table {{{ */

/** Table telling us for which context a group is valid */
static const int GroupValid[Event::C_LAST+1] = {
  /** valid groups for context Event::C_GENERIC */
  CTX(Event::G_INTERNAL)|CTX(Event::G_GENERIC)|CTX(Event::G_MOVE),
  /** valid groups for context Event::C_INDEX */
  CTX(Event::G_GENERIC)|CTX(Event::G_REPLY)|CTX(Event::G_EDIT),
  /** valid groups for context Event::C_PAGER */
  CTX(Event::G_GENERIC)|CTX(Event::G_REPLY)|CTX(Event::G_EDIT),
  /** valid groups for context Event::C_HELP */
  0,

  /** for which contexts G_LAST is valid */
  0
};
/* }}} */
/* group validity table #2 {{{ */

/** Table telling us for which group an event is valid */
static const Event::group GroupValid2[Event::E_LAST+1] = {
  /** valid groups for event Event::E_CONTEXT_ENTER */
  Event::G_INTERNAL,
  /** valid groups for event Event::E_CONTEXT_LEAVE */
  Event::G_INTERNAL,
  /** valid groups for event Event::E_CONTEXT_REENTER */
  Event::G_INTERNAL,
  /** valid groups for event Event::E_OPTION_CHANGE */
  Event::G_INTERNAL,
  /** valid groups for event Event::E_HELP */
  Event::G_GENERIC,
  /** valid groups for event Event::E_SHOW_VERSION */
  Event::G_GENERIC,
  /** valid groups for event Event::E_REPLY */
  Event::G_REPLY,
  /** valid groups for event Event::E_LIST_REPLY */
  Event::G_REPLY,
  /** valid groups for event Event::E_GROUP_REPLY */
  Event::G_REPLY,
  /** valid groups for event Event::E_PAGE_UP */
  Event::G_MOVE,
  /** valid groups for event Event::E_EDIT_MESSAGE */
  Event::G_EDIT,
  /** valid groups for event Event::E_BREAK_THREAD */
  Event::G_EDIT,
  /** valid groups for event Event::E_LINK_THREAD */
  Event::G_EDIT,

  /** for which group E_LAST is valid */
  Event::G_LAST
};
/* }}} */

Event::Event (Debug* debug) {
  int i = 0, j = 0;

  this->debug = debug;
  this->active = true;
  this->contextStack = new std::vector<Event::context>;

  for (i = 0; i < C_LAST; i++)
    for (j = 0; j < E_LAST; j++) {
      handlers[i][j] = NULL;
      bindings[i][j].key = NULL;
      bindings[i][j].name = NULL;
      bindings[i][j].help = NULL;
    }
}

Event::~Event (void) {
  int i = 0, j = 0;
  handle_t* handler = NULL;

  DEBUGPRINT(1,("cleanup event handler"));

  while (contextStack->size () != 0)
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
  delete (contextStack);
}

bool Event::init (void) {
  DEBUGPRINT(1,("init event handler"));
  bindings[C_INDEX][E_HELP].key = str_dup ("h");
  bindings[C_INDEX][E_HELP].defkey = "h";
  bindings[C_INDEX][E_HELP].name = EvStr[E_HELP];
  bindings[C_INDEX][E_HELP].help = EvHelp[E_HELP];
  bindings[C_PAGER][E_HELP].key = str_dup ("h");
  bindings[C_PAGER][E_HELP].defkey = "h";
  bindings[C_PAGER][E_HELP].name = EvStr[E_HELP];
  bindings[C_PAGER][E_HELP].help = EvHelp[E_HELP];
  bindings[C_GENERIC][E_SHOW_VERSION].key = str_dup ("V");
  bindings[C_GENERIC][E_SHOW_VERSION].defkey = "V";
  bindings[C_GENERIC][E_SHOW_VERSION].name = EvStr[E_SHOW_VERSION];
  bindings[C_GENERIC][E_SHOW_VERSION].help = EvHelp[E_SHOW_VERSION];
  bindings[C_INDEX][E_REPLY].key = str_dup ("r");
  bindings[C_INDEX][E_REPLY].defkey = "r";
  bindings[C_INDEX][E_REPLY].name = EvStr[E_REPLY];
  bindings[C_INDEX][E_REPLY].help = EvHelp[E_REPLY];
  bindings[C_PAGER][E_REPLY].key = str_dup ("r");
  bindings[C_PAGER][E_REPLY].defkey = "r";
  bindings[C_PAGER][E_REPLY].name = EvStr[E_REPLY];
  bindings[C_PAGER][E_REPLY].help = EvHelp[E_REPLY];
  bindings[C_INDEX][E_LIST_REPLY].key = str_dup ("L");
  bindings[C_INDEX][E_LIST_REPLY].defkey = "L";
  bindings[C_INDEX][E_LIST_REPLY].name = EvStr[E_LIST_REPLY];
  bindings[C_INDEX][E_LIST_REPLY].help = EvHelp[E_LIST_REPLY];
  bindings[C_PAGER][E_LIST_REPLY].key = str_dup ("L");
  bindings[C_PAGER][E_LIST_REPLY].defkey = "L";
  bindings[C_PAGER][E_LIST_REPLY].name = EvStr[E_LIST_REPLY];
  bindings[C_PAGER][E_LIST_REPLY].help = EvHelp[E_LIST_REPLY];
  bindings[C_INDEX][E_GROUP_REPLY].key = str_dup ("g");
  bindings[C_INDEX][E_GROUP_REPLY].defkey = "g";
  bindings[C_INDEX][E_GROUP_REPLY].name = EvStr[E_GROUP_REPLY];
  bindings[C_INDEX][E_GROUP_REPLY].help = EvHelp[E_GROUP_REPLY];
  bindings[C_PAGER][E_GROUP_REPLY].key = str_dup ("g");
  bindings[C_PAGER][E_GROUP_REPLY].defkey = "g";
  bindings[C_PAGER][E_GROUP_REPLY].name = EvStr[E_GROUP_REPLY];
  bindings[C_PAGER][E_GROUP_REPLY].help = EvHelp[E_GROUP_REPLY];
  bindings[C_GENERIC][E_PAGE_UP].key = str_dup ("pgup");
  bindings[C_GENERIC][E_PAGE_UP].defkey = "pgup";
  bindings[C_GENERIC][E_PAGE_UP].name = EvStr[E_PAGE_UP];
  bindings[C_GENERIC][E_PAGE_UP].help = EvHelp[E_PAGE_UP];
  bindings[C_INDEX][E_EDIT_MESSAGE].key = str_dup ("e");
  bindings[C_INDEX][E_EDIT_MESSAGE].defkey = "e";
  bindings[C_INDEX][E_EDIT_MESSAGE].name = EvStr[E_EDIT_MESSAGE];
  bindings[C_INDEX][E_EDIT_MESSAGE].help = EvHelp[E_EDIT_MESSAGE];
  bindings[C_PAGER][E_EDIT_MESSAGE].key = str_dup ("e");
  bindings[C_PAGER][E_EDIT_MESSAGE].defkey = "e";
  bindings[C_PAGER][E_EDIT_MESSAGE].name = EvStr[E_EDIT_MESSAGE];
  bindings[C_PAGER][E_EDIT_MESSAGE].help = EvHelp[E_EDIT_MESSAGE];
  bindings[C_INDEX][E_BREAK_THREAD].key = str_dup ("#");
  bindings[C_INDEX][E_BREAK_THREAD].defkey = "#";
  bindings[C_INDEX][E_BREAK_THREAD].name = EvStr[E_BREAK_THREAD];
  bindings[C_INDEX][E_BREAK_THREAD].help = EvHelp[E_BREAK_THREAD];
  bindings[C_INDEX][E_LINK_THREAD].key = str_dup ("&");
  bindings[C_INDEX][E_LINK_THREAD].defkey = "&";
  bindings[C_INDEX][E_LINK_THREAD].name = EvStr[E_LINK_THREAD];
  bindings[C_INDEX][E_LINK_THREAD].help = EvHelp[E_LINK_THREAD];

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
      return (false);
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
  DEBUGPRINT(D_EVENT,("enter ctx=%s from %s:%d", CtxStr[context],
                      NONULL (file), line));
  emit (context, E_CONTEXT_ENTER, NULL, false, data);
  contextStack->push_back (context);
}

void Event::_unsetContext (const char* file, int line, unsigned long data) {
  Event::context context;

  context = contextStack->at (contextStack->size ()-1);
  contextStack->pop_back ();
  DEBUGPRINT(D_EVENT,("leave ctx=%s from %s:%d", CtxStr[context],
                      NONULL (file), line));
  emit (context, E_CONTEXT_LEAVE, NULL, false, data);
  if (contextStack->size () != 0) {
    context = contextStack->at (contextStack->size ()-1);
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

const char* Event::getGroupName (Event::group group) {
  return (GroupStr[group]);
}

bool Event::eventValid (Event::context context, Event::event event) {
  return (EvValid[event] & CTX(context));
}

std::vector<Event::group>* Event::getGroups (Event::context context) {
  int i = 0;
  std::vector<Event::group>* ret = new std::vector<Event::group>;

  for (i = 0; i < Event::G_LAST; i++)
    if (GroupValid[context] & CTX(i))
      ret->push_back ((Event::group) i);

  if (ret->size () == 0) {
    delete (ret);
    return (NULL);
  }

  return (ret);
}

std::vector<Event::event>* Event::getEvents (Event::context context, Event::group group) {
  int i = 0;
  std::vector<Event::event>* ret = new std::vector<Event::event>;

  for (i = 0; i < (int) Event::E_LAST; i++)
    /*
     * - for all events, check wheter they're part of a context
     * - if event valid for context, check group, too
     */
    if (EvValid[i] & CTX(context) && (GroupValid2[(Event::event) i] == group))
      ret->push_back ((Event::event) i);

  if (ret->size () == 0) {
    delete (ret);
    return (NULL);
  }

  return (ret);
}

binding_t* Event::getHelp (Event::context context, Event::event event) {
  return (&bindings[context][event]);
}
