/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/rfc2047_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Signal unit tests
 *
 * This file is published under the GNU General Public License.
 */
#include <iostream>
#include <unit++/unit++.h>
#include "rfc2047_tests.h"

#include "core/core.h"
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
  { "€¤", "=?utf-8?q?=E2=82=AC=C2=A4?=", true },
  { "test ä und üä", "test =?UTF-8*EN?Q?=C3=A4?= und =?iso-8859-1?Q?=FC?=           =?UTF-8*EN?Q?=C3=A4?=", true },
  { "üüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüüü",
    "=?iso-8859-1?Q?"
    "=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC"
    "=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC"
    "=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC=FC"
    "?=", true },

  { NULL, NULL, false }
};

size_t rfc2047_tests::isclean(buffer_t* src) {
  register size_t i = 0;
  for (i = 0; !(src->str[i] & 0x80) && i < src->len; i++);
  return i;
}

void rfc2047_tests::test_decode_buf (buffer_t* dst) {
  int i;
  buffer_t src, msg;

  buffer_init(&src);
  buffer_init(&msg);
  for (i = 0; Tests[i].dec; i++) {
    buffer_shrink(&src,0);
    buffer_shrink(&msg,0);
    buffer_add_str(&src,Tests[i].enc,-1);
    buffer_add_str(&msg,"\ndecoding:\n",-1);
    buffer_add_buffer(&msg,&src);
    buffer_add_ch(&msg, '\n');
    assert_true(msg.str,rfc2047_decode(dst,&src,"utf-8")==Tests[i].valid);
    if (!dst)
      continue;
    buffer_add_str(&msg,"to:\n",4);
    buffer_add_str(&msg,Tests[i].dec,-1);
    buffer_add_str(&msg,"\nworked:\n",9);
    buffer_add_buffer(&msg,dst);
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

  buffer_t tmp;
  buffer_init(&tmp);
  for (int i = 0; Tests[i].enc; i++) {
    buffer_shrink(&tmp,0);
    buffer_shrink(&dst,0);
    buffer_add_str(&tmp,Tests[i].dec,-1);
    rfc2047_encode(&dst,&tmp,"utf-8");
    assert_eq("encoded buffer is ASCII only",isclean(&dst),dst.len);
  }
}

rfc2047_tests::rfc2047_tests() : suite("rfc2047_tests") {
  core_init();
  add("rfc2047",testcase(this,"test_rfc2047_decode",
                         &rfc2047_tests::test_decode));
}

rfc2047_tests::~rfc2047_tests() {
  core_cleanup();
}
