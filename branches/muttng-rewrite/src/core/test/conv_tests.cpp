/** @ingroup core_unit */
/**
 * @file core/test/conv_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: conversion unit tests
 */
#include <unit++/unit++.h>
#include <iostream>
#include "conv.h"
#include "str.h"
#include "conv_tests.h"

using namespace unitpp;

static const char* ToSets[] = {
  "utf-8", "iso-8859-15",
  NULL
};

static const char* TestStrings[] = {
  "teststring", "ümläut",
  NULL
};

void conv_tests::test_iconv() {
  buffer_t orig, conv, conv2;
  int i = 0, a = 0;

  buffer_init(&orig);
  buffer_init(&conv);
  buffer_init(&conv2);

  /** For different character sets and test strings, we: */
  for (a = 0; ToSets[a]; a++) {
    for (i = 0; TestStrings[i]; i++) {
      buffer_shrink(&orig,0);
      buffer_shrink(&conv,0);
      buffer_shrink(&conv2,0);
      buffer_add_str(&orig,TestStrings[i],-1);
      buffer_add_buffer(&conv,&orig);
      /** - see if conv_iconv() works at all */
      assert_true("iconv() succeeds",conv_iconv(&conv,"utf-8",ToSets[a]));
      buffer_add_buffer(&conv2,&conv);
      /** - convert result back */
      conv_iconv(&conv2,ToSets[a],"utf-8");
      /** - and see if it's identical to the original */
      assert_true("iconv() reverse worked",buffer_equal2(&orig,&conv2));
    }
  }
  buffer_free(&orig);
  buffer_free(&conv);
  buffer_free(&conv2);
}

conv_tests::conv_tests() : suite("conv_tests") {
  add("iconv",testcase(this,"test_iconv",&conv_tests::test_iconv));
}
