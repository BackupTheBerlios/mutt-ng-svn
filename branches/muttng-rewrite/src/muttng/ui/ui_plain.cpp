/** @ingroup muttng_ui_cli */
/**
 * @file muttng/ui/ui_plain.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: CLI user interface
 */
#include <iostream>

#include "ui_plain.h"

#include "core/mem.h"
#include "core/str.h"

using namespace std;

UIPlain::UIPlain (void) {}
UIPlain::~UIPlain (void) {}

bool UIPlain::start (void) { return (true); }
bool UIPlain::end (void) { return (true); }
bool UIPlain::enterFilename (void) { return (true); }
bool UIPlain::enterPassword (void) { return (true); }
bool UIPlain::answerQuestion (void) { return (true); }

void UIPlain::displayError (const char* message) {
  if (message)
    cerr << message << endl;
}

void UIPlain::displayMessage (const char* message) {
  if (message)
    cout << message << endl;
}

bool UIPlain::enterValue(buffer_t* dst, buffer_t* prompt, size_t dstlen) {
  char* buf = (char*) mem_malloc(dstlen+1);
  buf[dstlen] = '\0';
  std::cout<<prompt->str<<std::endl;
  std::cin.getline(buf,dstlen);
  size_t len = str_len(buf);
  buffer_add_str(dst,buf,-1);
  mem_free(&buf);
  return len>0;
}

bool UIPlain::enterPassword(buffer_t* dst, buffer_t* prompt, size_t dstlen) {
  return enterValue(dst,prompt,dstlen);
}
