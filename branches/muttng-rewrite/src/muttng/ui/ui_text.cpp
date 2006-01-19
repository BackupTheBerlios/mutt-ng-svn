/** @ingroup muttng_ui */
/**
 * @file muttng/ui/ui_text.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Text-based UI base class
 *
 * This file is published under the GNU General Public License.
 */
#include <iostream>

#include "ui_text.h"

using namespace std;

UIText::UIText (void) {}
UIText::~UIText (void) {}

void UIText::displayVersion (const char* name, const char* copyright,
                             const char* options, const char* reach) {
  if (name)
    cout << name << "\n";
  if (copyright)
    cout << copyright << "\n";
  if (options)
    cout << options << "\n";
  if (reach)
    cout << reach << "\n";
}

void UIText::displayWarranty (const char* name, const char* copyright,
                              const char* license, const char* reach) {
  if (name)
    cout << name << "\n";
  if (copyright)
    cout << copyright << "\n";
  if (license)
    cout << license << "\n";
  if (reach)
    cout << reach << "\n";
}
