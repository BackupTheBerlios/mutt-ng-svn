/** @ingroup muttng_event */
/**
 * @file muttng/event/event.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Event handling
 */
#include "core/mem.h"
#include "core/str.h"

#include "event.h"

typedef struct {
  eventhandler_t* handler;
  void* self;
  bool input;
} handle_t;

static const char* CtxStr[Event::C_LAST+1] = {
#include "contextlist.cpp"
  NULL
};

static const char* EvStr[Event::E_LAST+1] = {
#include "eventlist.cpp"
  NULL
};

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
  DEBUGPRINT(1,("cleanup event handler"));
  for (i = 0; i < C_LAST; i++)
    for (j = 0; j < E_LAST; j++)
      list_del (&handlers[i][j], (list_del_t*) _mem_free);
  list_del (&contextStack, NULL);
}

bool Event::init (void) {
  DEBUGPRINT(1,("init event handler"));
  setContext (C_GENERIC, 0);
  return (true);
}

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
}

bool Event::_emit (const char* file, int line, Event::context context, Event::event event,
                  const char* input, bool complete, unsigned long data) {
  list_t* list = NULL;
  int i = 0;
  handle_t* handle = NULL;
  Event::state state = S_OK;

  DEBUGPRINT(2,("emit: ctx=%s, ev=%s, in='%s', compl=%d, d=0x%x from %s:%d",
                CtxStr[context], EvStr[event], NONULL (input), complete,
                data, NONULL (file), line));

  if (!active || !(list = this->handlers[context][event]))
    return (false);
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
  DEBUGPRINT(2,("enter ctx=%s from %s:%d", CtxStr[context], NONULL (file), line));
  emit (context, E_CONTEXT_ENTER, NULL, false, data);
  list_push_back (&contextStack, (LIST_ITEMTYPE) context);
}

void Event::_unsetContext (const char* file, int line, unsigned long data) {
  Event::context context;

  context = (Event::context) list_pop_back (contextStack);
  DEBUGPRINT(2,("leave ctx=%s from %s:%d", CtxStr[context], NONULL (file), line));
  emit (context, E_CONTEXT_LEAVE, NULL, false, data);
  if (!list_empty (contextStack)) {
    context = (Event::context) contextStack->data[contextStack->length-1];
    DEBUGPRINT(2,("re-enter ctx=%s from %s:%d", CtxStr[context], NONULL (file), line));
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
  DEBUGPRINT(2,("event handler disabled"));
}

void Event::enable (void) {
  DEBUGPRINT(2,("event handler enabled"));
  active = true;
}

const char* Event::getContextName (Event::context context) {
  return (CtxStr[context]);
}

const char* Event::getEventName (Event::event event) {
  return (EvStr[event]);
}
