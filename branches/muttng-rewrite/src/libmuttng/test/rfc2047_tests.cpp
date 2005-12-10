/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/rfc2047_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Signal unit tests
 */
#include <iostream>
#include <unit++/unit++.h>
#include "rfc2047_tests.h"

#include "core/buffer.h"
#include "core/str.h"
#include "libmuttng/util/rfc2047.h"

using namespace unitpp;

/** Tests for rfc2047_decode(). */
static struct {
  /** hand-decoded string */
  char* dec;
  /** hand-encoded string */
  char* enc;
  /** whether encoded one is valid */
  bool valid;
} Tests[] = {
  { "ascii text", "ascii text", true },
  { "€", "=?UtF-8*de?q?=E2=82=AC?=", true },
  { "test ä und üä", "test =?UTF-8*EN?Q?=C3=A4?= und =?iso-8859-1?Q?=FC?=           =?UTF-8*EN?Q?=C3=A4?=", true },

  { NULL, NULL, false }
};

void rfc2047_tests::test_decode_buf (buffer_t* dst) {
  int i;
  buffer_t src, msg;

  buffer_init(&src);
  buffer_init(&msg);
  for (i = 0; Tests[i].dec; i++) {
    buffer_shrink(&src,0);
    buffer_shrink(&msg,0);
    buffer_add_str(&src,Tests[i].enc,-1);
    buffer_add_str(&msg,"decoding '",-1);
    buffer_add_buffer(&msg,&src);
    buffer_add_ch(&msg, '\'');
    assert_true(msg.str,rfc2047_decode(dst,&src,"utf-8")==Tests[i].valid);
    if (!dst)
      continue;
    buffer_add_str(&msg," to '",5);
    buffer_add_str(&msg,Tests[i].dec,-1);
    buffer_add_str(&msg,"' worked: '",11);
    buffer_add_buffer(&msg,dst);
    buffer_add_ch(&msg,'\'');
    assert_true(msg.str,buffer_equal1(dst,Tests[i].dec,-1));
  }

  buffer_free(&src);
  buffer_free(&msg);
}

void rfc2047_tests::test_decode () {
  buffer_t dst;
  buffer_init(&dst);
  test_decode_buf(&dst);
  buffer_free(&dst);
  test_decode_buf(NULL);
}

rfc2047_tests::rfc2047_tests() : suite("rfc2047_tests") {
  add("rfc2047",testcase(this,"test_rfc2047_decode",
                         &rfc2047_tests::test_decode));
}
