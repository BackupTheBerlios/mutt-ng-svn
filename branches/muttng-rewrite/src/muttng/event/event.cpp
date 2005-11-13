/**
 * @ingroup muttng_event
 */
/**
 * @file muttng/event/event.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Event handling implementation
 */
#include "event.h"

Event::Event (Debug* debug) { this->debug = debug; }
Event::~Event (void) {}

bool Event::init (void) {
  DEBUGPRINT(1,("init event handler"));
  return (true);
}

void Event::bindInternal (Event::context context,
                          Event::event event,
                          bool input,
                          Event::state (*handler) (Event::context context,
                                                   Event::event event,
                                                   const char* input,
                                                   bool complete,
                                                   unsigned long data)) {
  (void) context;
  (void) event;
  (void) input;
  (void) handler;
}

bool Event::bindUser (Event::context context, const char* key,
                      const char* func, buffer_t* error) {
  (void) context;
  (void) key;
  (void) func;
  (void) error;
  return (true);
}

void Event::setContext (Event::context, unsigned long data) {
  enqueue (E_CONTEXT_ENTER, data);
  /* push context onto currentContext */
}

void Event::unsetContext (unsigned long data) {
  /** pop Context from currentContext */
  enqueue (E_CONTEXT_LEAVE, data);
}

void Event::enqueue (Event::event event, unsigned long data) {
  (void) event;
  (void) data;
}
