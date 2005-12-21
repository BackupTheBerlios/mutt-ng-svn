/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/lib_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Libmuttng base unit testing class
 */
#include "lib_tests.h"

#include <iostream>

lib_tests::lib_tests() {
  connectSignal(displayProgress,this,&lib_tests::displayText);
  connectSignal(displayMessage,this,&lib_tests::displayText);
  connectSignal(displayError,this,&lib_tests::displayText);
}

lib_tests::~lib_tests() {}

bool lib_tests::displayText (const buffer_t* message) {
  if (message) std::cerr<<message->str<<std::endl;
  return true;
}
