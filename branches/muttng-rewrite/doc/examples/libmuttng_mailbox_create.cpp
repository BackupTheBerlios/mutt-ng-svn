
/**
 * @file examples/libmuttng_mailbox_create.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Example: how to create mailbox from URL
 */
#include <iostream>
#include "core/buffer.h"
#include "libmuttng/mailbox/mailbox.h"

/**
 * @c main().
 * @param argc Counter for command-line arguments.
 * @param argv Command-line arguments.
 * @return 0 on success, 1 otherwise.
 */
int main (int argc, char** argv) {
  buffer_t error;
  Mailbox* folder = NULL;

  buffer_init(&error);

  if (argc >= 2) {
    if (!(folder = Mailbox::fromURL (argv[1], &error))) {
      /* error: likely URL is wrong */
      std::cerr << "Error opening folder '" << argv[1] <<
        "': " << (error.str?error.str:"") << std::endl;
      buffer_free(&error);
      return 1;
    } else {
      /* success */
      std::cout << "success" << std::endl;
      delete folder;
    }
  } else {
    std::cout << "Usage: " << argv[0] << " [url]" << std::endl;
  }
  return 0;
};
