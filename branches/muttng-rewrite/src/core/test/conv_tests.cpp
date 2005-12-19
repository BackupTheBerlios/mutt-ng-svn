/** @ingroup core_unit */
/**
 * @file core/test/conv_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: conversion unit tests
 */
#include <unit++/unit++.h>
#include <iostream>

#include "core.h"
#include "conv.h"
#include "str.h"
#include "conv_tests.h"

using namespace unitpp;

/** character sets we do test conversions with */
static const char* ToSets[] = {
  /** no comment */
  "utf-8",
  /** for euro currency sign */
  "iso-8859-15",
  /** for international currency sign but no euro */
  "iso-8859-1",
  /** @c conv.c: alias for iso-8859-8 */
  "hebrew",
  /** @c conv.c: alias for iso-8859-1 */
  "latin1",
  /** @c conv.c: alias for iso-8859-6 */
  "csISOLatinArabic",
  NULL
};

/** UTF-8 test strings */
static struct {
  /** test string */
  const char* str;
  /** whether we expect a loss, i.e. non-ASCII input */
  int loss;
} TestStrings[] = {
  { "teststring", 0 },
  { "ümläut", 1 },
  { "Täst mit ¤ ünd €", 1 },
  { NULL, 0 }
};

void conv_tests::test_iconv() {
  buffer_t orig, conv, conv2, msg;
  int i = 0, a = 0;

  buffer_init(&orig);
  buffer_init(&conv);
  buffer_init(&conv2);
  buffer_init(&msg);

  /** For different character sets and test strings, we: */
  for (a = 0; ToSets[a]; a++) {
    for (i = 0; TestStrings[i].str; i++) {
      buffer_shrink(&orig,0);
      buffer_shrink(&conv,0);
      buffer_shrink(&conv2,0);
      buffer_shrink(&msg,0);
      buffer_add_str(&orig,TestStrings[i].str,-1);
      buffer_add_buffer(&conv,&orig);
      /**
       * - see if conv_iconv() works at all:
       *        -# if conv_iconv() succeeds: okay
       *        -# if conv_iconv() fails: the utf-8 string must be
       *           marked not fully convertable and we don't convert
       *           from utf-8 to utf-8 currently (since that musn't
       *           produce a loss)
       */
      assert_true("iconv() succeeds",conv_iconv(&conv,"utf-8",ToSets[a]) ||
                  (TestStrings[i].loss && !str_eq(ToSets[a],"utf-8")));
      buffer_add_buffer(&conv2,&conv);
      /** - convert result back */
      conv_iconv(&conv2,ToSets[a],"utf-8");
      /** - and see if it's identical to the original */
      buffer_add_str(&msg,"reverse iconv(): str='",-1);
      buffer_add_str(&msg,TestStrings[i].str,-1);
      buffer_add_str(&msg,"' loss=",-1);
      buffer_add_snum(&msg,TestStrings[i].loss,-1);
      buffer_add_str(&msg," toset=",-1);
      buffer_add_str(&msg,ToSets[a],-1);
      buffer_add_str(&msg," got='",-1);
      buffer_add_buffer(&msg,&conv2);
      buffer_add_ch(&msg,'\'');
      /*
       * the logic here is a little complicated:
       * - if reverse conversion is equal with orig -> okay
       * - if not:
       *        - if we know we don't have a loss (since string is plain ASCII)
       *          _OR_ we do UTF-8<->UTF-8 on a string, original and
       *          conversion must be equal
       *        - if we have a loss we assume iconv() has placed '?'
       *          in the string somewhere to mark failure
       */
      assert_true(msg.str,buffer_equal2(&orig,&conv2) ||
                          (((!TestStrings[i].loss || str_eq("utf-8",ToSets[a])) &&
                            buffer_equal2(&orig,&conv2)) ||
                          (TestStrings[i].loss && strchr(conv2.str,'?'))));
    }
  }
  buffer_free(&orig);
  buffer_free(&conv);
  buffer_free(&conv2);
}

conv_tests::conv_tests() : suite("conv_tests") {
  core_init();
  add("iconv",testcase(this,"test_iconv",&conv_tests::test_iconv));
}

conv_tests::~conv_tests() {
  core_cleanup();
}
