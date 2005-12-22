/** @ingroup core_unit */
/**
 * @file core/test/net_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: IDNA unit tests
 */
#include <unit++/unit++.h>
#include <iostream>

#include "core.h"
#include "conv.h"
#include "str.h"
#include "net.h"
#include "net_tests.h"

using namespace unitpp;

static struct {
  const char* utf8;
  const char* latin1;
  const char* puny;
  bool full;
} Tests[] = {
  { "müller.de", "m\xfcller.de", "xn--mller-kva.de", true },
  /* from en.wikipedia.org */
  { "tūdaliņ.lv", NULL, "xn--tdali-d8a8w.lv", false },
  { NULL, NULL, NULL, false }
};

void net_tests::test_to_local () {
  unsigned short i;
  buffer_t src,dst;

  buffer_init(&src);
  buffer_init(&dst);

  for (i = 0; Tests[i].utf8; i++) {
    buffer_shrink(&src,0);
    buffer_add_str(&src,Tests[i].puny,-1);

    buffer_free(&dst); buffer_init(&dst);
    assert_true("net_host_to_local(utf8) works #1",net_host_to_local(&dst,&src,"utf-8",0));
    assert_true("net_host_to_local(utf8) works #2",buffer_equal1(&dst,Tests[i].utf8,-1));

    buffer_free(&dst); buffer_init(&dst);
    assert_true("net_host_to_local(latin1) works #1",net_host_to_local(&dst,&src,"latin1",0)==Tests[i].full);
    assert_true("net_host_to_local(latin1) works #2",buffer_equal1(&dst,Tests[i].latin1,-1)==Tests[i].full);
  }

  buffer_free(&src);
  buffer_free(&dst);
}

void net_tests::test_from_local () {
  unsigned short i;
  buffer_t src,dst;

  buffer_init(&src);
  buffer_init(&dst);

  for (i = 0; Tests[i].utf8; i++) {

    buffer_shrink(&src,0); buffer_add_str(&src,Tests[i].utf8,-1);
    buffer_free(&dst); buffer_init(&dst);
    assert_true("net_local_to_host(utf8) works #1",net_local_to_host(&dst,&src,"utf-8"));
    assert_true("net_local_to_host(utf8) works #2",buffer_equal1(&dst,Tests[i].puny,-1));

    if (!Tests[i].full || !Tests[i].latin1) 
      continue;

    buffer_shrink(&src,0); buffer_add_str(&src,Tests[i].latin1,-1);
    buffer_free(&dst); buffer_init(&dst);
    assert_true("net_local_to_host(utf8) works #1",net_local_to_host(&dst,&src,"latin1"));
    assert_true("net_local_to_host(utf8) works #2",buffer_equal1(&dst,Tests[i].puny,-1));
  }

  buffer_free(&src);
  buffer_free(&dst);
}

net_tests::net_tests() : suite("net_tests") {
  core_init();
  add("net",testcase(this,"test_to_local",&net_tests::test_to_local));
  add("net",testcase(this,"test_from_local",&net_tests::test_from_local));
}

net_tests::~net_tests() {
  core_cleanup();
}
