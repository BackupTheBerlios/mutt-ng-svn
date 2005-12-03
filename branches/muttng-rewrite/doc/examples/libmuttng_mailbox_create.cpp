
/* Example: how to create mailbox from URL */
#include "core/buffer.h"
#include "libmuttng/mailbox/mailbox.h"

class Test {
  void test() {
    buffer_t error;
    Mailbox* folder = NULL;
    buffer_init(&error);
    if (!(folder = Mailbox::fromURL ("file:///dev/null", &error))) {
      /* error: likely URL is wrong */
    } else {
      /* success */
    }
  }
};
