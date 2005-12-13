
/**
 * @file examples/libmuttng_url.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Example: how to parsr a string into an URL
 */
#include <iostream>
#include "core/buffer.h"
#include "libmuttng/util/url.h"

/**
 * @c main().
 * @param argc Counter for command-line arguments.
 * @param argv Command-line arguments.
 * @return 0 on success, 1 otherwise.
 */
int main (int argc, char** argv) {
  url_t* url = NULL;
  buffer_t error;

  buffer_init(&error);
  if (argc >= 2) {
    if (!(url = url_from_string(argv[1],&error))) {
      std::cerr << "URL '" << argv[1] << "' is invalid: "
        << error.str << std::endl;
      buffer_free(&error);
      return 1;
    }
    std::cout << "URL is " << (url->secure?"":"not ")
      << "secure" << std::endl;
    url_free(url);
  } else {
    std::cout << "Usage: " << argv[0] << " [url]" << std::endl;
  }
  return (0);
}
