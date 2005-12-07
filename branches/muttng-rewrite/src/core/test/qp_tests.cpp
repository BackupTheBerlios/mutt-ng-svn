#include <iostream>

#include "qp.h"
#include "qp_tests.h"

static struct {
  const char * decoded;
  const char * encoded;
} ECTable[] = {
  { "asdf", "asdf" },
  { "", "" },
  { " ", "=20" },
  { "This is quite a long text, just to test very long text, which is always a good idea to do in unit tests.", "This=20is=20quite=20a=20long=20text=2C=20just=20to=20test=20very=20long=20text=2C=20which=20is=20always=20a=20good=20idea=20to=20do=20in=20unit=20tests=2E" },
  { NULL, NULL } // end of table
};

qp_tests::qp_tests() : suite("qp_tests") {
  add("qp",testcase(this,"test_encode",&qp_tests::test_encode));
  add("qp",testcase(this,"test_decode",&qp_tests::test_decode));
  add("qp",testcase(this,"test_both",&qp_tests::test_both));
}

qp_tests::~qp_tests() { }

void qp_tests::test_encode() {
  int i;
  buffer_t dec, enc;
  buffer_t tmp, msg;
  buffer_init(&dec); buffer_init(&enc); buffer_init(&tmp); buffer_init(&msg);

  for (i=0;ECTable[i].decoded;++i) {
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); buffer_shrink(&tmp,0); buffer_shrink(&msg,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    qp_encode(&tmp,&dec,'=');

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' encodes to '",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,"'",-1);

    assert_true(msg.str,buffer_equal2(&tmp,&enc));
  }
}

void qp_tests::test_decode() {
  int i;
  buffer_t dec, enc;
  buffer_t tmp, msg;
  buffer_init(&dec); buffer_init(&enc); buffer_init(&tmp); buffer_init(&msg);

  for (i=0;ECTable[i].decoded;++i) {
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); buffer_shrink(&tmp,0); buffer_shrink(&msg,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    qp_decode(&tmp,&enc,'=',NULL);

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,"' decodes to '",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"'",-1);

    assert_true(msg.str,buffer_equal2(&tmp,&dec));
  }
}

void qp_tests::test_both() {
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
    qp_encode(&tmp,&dec,'=');
    /* then decode back */
    qp_decode(&tmp2,&tmp,'=',NULL);

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' encodes and decodes back correctly",-1);

    assert_true(msg.str,buffer_equal2(&tmp2,&dec));
  }
}
