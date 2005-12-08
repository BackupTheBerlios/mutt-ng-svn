#include <iostream>

#include "qp.h"
#include "qp_tests.h"

/** table with test strings */
static struct {
  /** UTF-8 string */
  const char * decoded;
  /** QP-hand-encoded */
  const char * encoded;
  /** if hand-encoded is valid. */
  int valid;
} ECTable[] = {
  { "asdf", "asdf", 1 },
  { "", "", 1 },
  { " ", "=20", 1 },
  { "Täst mit ¤ ünd €", "T=C3=A4st=20mit=20=C2=A4=20=C3=BCnd=20=E2=82=AC", 1 },
  /* '=Alid$' part is invalid */
  { "This is invalid.", "This=20is=20invalid=2E=Alid", 0 },
  /* check if encoding magic chars works/must not work */
  { "1+1=4", "1=2B1=3D4", 1 },
  { "-1%7", "=2D1=257", 1 },
  { "this is valid", "this=20is=20valid=00but=20not=20this", 0 },
  { NULL, NULL, 0 } // end of table
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
    if(!ECTable[i].valid)
      continue;
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); buffer_shrink(&tmp,0); buffer_shrink(&msg,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    qp_encode(&tmp,&dec,'=');

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' encodes to '",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,"' (got: '",-1);
    buffer_add_buffer(&msg,&tmp);
    buffer_add_str(&msg,"')",2);

    assert_true(msg.str,buffer_equal2(&tmp,&enc));
  }
}

void qp_tests::test_decode() {
  int i;
  size_t l;
  buffer_t dec, enc;
  buffer_t tmp, msg;
  buffer_init(&dec); buffer_init(&enc); buffer_init(&tmp); buffer_init(&msg);

  for (i=0;ECTable[i].decoded;++i) {
    buffer_shrink(&dec,0); buffer_shrink(&enc,0); buffer_shrink(&tmp,0); buffer_shrink(&msg,0);
    buffer_add_str(&dec,ECTable[i].decoded,-1);
    buffer_add_str(&enc,ECTable[i].encoded,-1);

    buffer_add_str(&msg,"decode(",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,")==",-1);
    buffer_add_num(&msg,ECTable[i].valid,-1);
    assert_true(msg.str,qp_decode(&tmp,&enc,'=',&l)==ECTable[i].valid);
    buffer_shrink(&msg,0);

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&enc);
    buffer_add_str(&msg,"' decodes to '",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' (got: '",-1);
    buffer_add_buffer(&msg,&tmp);
    buffer_add_str(&msg,"')",2);

    assert_true(msg.str,buffer_equal1(&tmp,dec.str,l));
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
    assert_true("decode works",qp_decode(&tmp2,&tmp,'=',NULL));

    buffer_add_str(&msg,"'",-1);
    buffer_add_buffer(&msg,&dec);
    buffer_add_str(&msg,"' encodes and decodes back correctly",-1);

    assert_true(msg.str,buffer_equal2(&tmp2,&dec));
  }
}
