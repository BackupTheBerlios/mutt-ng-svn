
/* Example: how to parse string into URL */
#include <iostream>
#include "core/buffer.h"
#include "libmuttng/util/url.h"

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
