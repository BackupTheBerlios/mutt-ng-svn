/** @ingroup core_unit */
/**
 * @file core/test/conv_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: conversion unit tests
 */
#ifndef CORE_TEST_CONV__H
#define CORE_TEST_CONV__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * conversion unit testing class
 */
class conv_tests : public suite {
  /**
   * Test for conv_iconv().
   * @test conv_iconv().
   */
  void test_iconv();
  public:
    conv_tests();
};

#endif
