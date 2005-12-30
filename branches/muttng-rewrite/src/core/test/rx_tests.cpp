/** @ingroup core_unit */
/**
 * @file core/test/rx_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Regular expression unit tests
 */
#include <unit++/unit++.h>
#include <iostream>

#include "core.h"
#include "conv.h"
#include "str.h"
#include "rx.h"
#include "rx_tests.h"

using namespace unitpp;

/** table of regexs */
static struct {
  /** pattern string */
  const char* pattern;
  /** whether pattern is valid regex */
  bool valid;
  /** test string */
  const char* subject;
  /** whether string matches regex */
  bool match;
} RX[] = {
  { "", true, "", false },
  { "^[", false, "l33t", false },

  { "^[.]+$", true, "...", true },

  { "^[^ ]*[ ]", true, ".", false },
  { "^[^ ]*[ ]", true, ". ", true },

  { "^foo$", true, "foobar", false },
  { "^foo$", true, "foo", true },

  { "^foo(baz|bar)$", true, "foobar", true },
  { "^foo(baz|bar)$", true, "foobaz", true },

  { "^foo.*(baz|bar|())$", true, "foobar", true },
  { "^foo.*(baz|bar|())$", true, "foo", true },
  { "^foo.*(baz|bar|())$", true, "foobaz", true },

  { "^foo.+(baz|bar|())$", true, "foo ", true },
  { "^foo.+(baz|bar|())$", true, "foo   bar", true },

  { "^foo(baz|bar)?", true, "foo ", true },
  { "^foo(baz|bar)?", true, "foobar", true },

  { NULL, false, NULL, false },
};

void rx_tests::test_rx_compile() {
  unsigned short i;
  buffer_t msg;
  buffer_init(&msg);
  for (i=0; RX[i].pattern; i++) {
    buffer_shrink(&msg,0);
    rx_t* rx = rx_compile(RX[i].pattern,NULL,0);
    buffer_add_str(&msg,"rx_compile(",-1);
    buffer_add_str(&msg,RX[i].pattern,-1);
    buffer_add_str(&msg,") works",-1);
    assert_true(msg.str,(RX[i].valid && rx!=NULL) || (!RX[i].valid && rx==NULL));
    rx_free(rx);
    assert_true("rx_free() works",rx==NULL || (rx->pattern==NULL && rx->rx==NULL));
  }
  buffer_free(&msg);
}

void rx_tests::test_rx_match() {
  unsigned short i;
  buffer_t msg;
  buffer_init(&msg);
  for (i=0; RX[i].pattern; i++) {
    buffer_shrink(&msg,0);
    rx_t* rx = rx_compile(RX[i].pattern,NULL,0);
    if (!RX[i].valid || !RX[i].match) continue;
    buffer_add_str(&msg,"rx_match(",-1);
    buffer_add_str(&msg,RX[i].pattern,-1); buffer_add_ch(&msg,',');
    buffer_add_str(&msg,RX[i].subject,-1); buffer_add_str(&msg,") works",-1);
    assert_eq(msg.str,1,rx_match(rx,RX[i].subject));
    rx_free(rx);
  }
  buffer_free(&msg);
}

rx_tests::rx_tests() : suite("rx_tests") {
  core_init();
  add("rx",testcase(this,"test_rx_compile",&rx_tests::test_rx_compile));
  add("rx",testcase(this,"test_rx_match",&rx_tests::test_rx_match));
}

rx_tests::~rx_tests() {
  core_cleanup();
}
