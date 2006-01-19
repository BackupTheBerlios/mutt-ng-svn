/** @ingroup libmuttng_unit */
/**
 * @file core/test/base64_tests.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: base64 unit tests
 *
 * This file is published under the GNU General Public License.
 */
#include "buffer.h"
#include "base64_tests.h"

/** table with test strings */
static struct {
  /** test string */
  const char * decoded;
  /** hand-encoded version of test string */
  const char * encoded;
} ECTable[] = {
  { "asdf", "YXNkZg==" },
  { "", "" },
  { " ", "IA==" },
  { "This is quite a long text, just to test very long text, which is always a good idea to do in unit tests.", "VGhpcyBpcyBxdWl0ZSBhIGxvbmcgdGV4dCwganVzdCB0byB0ZXN0IHZlcnkgbG9uZyB0ZXh0LCB3aGljaCBpcyBhbHdheXMgYSBnb29kIGlkZWEgdG8gZG8gaW4gdW5pdCB0ZXN0cy4=" },
  { NULL, NULL } // end of table
};

base64_tests::base64_tests() : suite("base64_tests") {
  add("base64",testcase(this,"test_encode",&base64_tests::test_encode));
  add("base64",testcase(this,"test_decode",&base64_tests::test_decode));
  add("base64",testcase(this,"test_both",&base64_tests::test_both));
}

base64_tests::~base64_tests() { }

void base64_tests::test_encode() {
  int i;
  buffer_t dec, enc;
  buffer_t tmp, msg;
  buffer_init(&dec); buffer_init(&enc); buffer_init(&tmp); buffer_init(&msg);

  for (i=0;ECTable[i].decoded;++i) {
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); buffer_shrink(&tmp,0); buffer_shrink(&msg,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    buffer_base64_encode(&tmp,&dec);

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' encodes to '",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,"'",-1);

    assert_true(msg.str,buffer_equal2(&tmp,&enc));
  }
}

void base64_tests::test_decode() {
  int i;
  buffer_t dec, enc;
  buffer_t tmp, msg;
  buffer_init(&dec); buffer_init(&enc); buffer_init(&tmp); buffer_init(&msg);

  for (i=0;ECTable[i].decoded;++i) {
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); buffer_shrink(&tmp,0); buffer_shrink(&msg,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    assert_true("decode works",buffer_base64_decode(&tmp,&enc,NULL));

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,"' decodes to '",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"'",-1);

    assert_true(msg.str,buffer_equal2(&tmp,&dec));
  }
}

void base64_tests::test_both() {
  int i;
  buffer_t dec, enc;
  buffer_t tmp, tmp2, msg;

  buffer_init(&dec); buffer_init(&enc); 
  buffer_init(&tmp); buffer_init(&msg); buffer_init(&tmp2);

  for (i=0;ECTable[i].decoded;++i) {
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); 
    buffer_shrink(&tmp,0); buffer_shrink(&msg,0); buffer_shrink(&tmp2,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    /* first encode */
    buffer_base64_encode(&tmp,&dec);
    /* then decode back */
    assert_true("decode works",buffer_base64_decode(&tmp2,&tmp,NULL));

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' encodes and decodes back correctly",-1);

    assert_true(msg.str,buffer_equal2(&tmp2,&dec));
  }
}
