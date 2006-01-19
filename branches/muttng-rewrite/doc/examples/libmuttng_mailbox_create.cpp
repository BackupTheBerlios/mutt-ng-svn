
/**
 * @file examples/libmuttng_mailbox_create.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Example: how to create mailbox from URL
 *
 * This file is published as public domain.
 */
#include <iostream>
#include "core/buffer.h"
#include "libmuttng/libmuttng.h"
#include "libmuttng/mailbox/mailbox.h"

/** "Minimal" client for libmuttng: just catch messages */
class MinClient {
  private:
    /**
     * Handler connected to message signals: just print
     * on stdout.
     * @param msg Message.
     * @return true
     */
    bool print (const buffer_t* msg) {
      std::cout<<msg->str<<std::endl;
      return true;
    }
    /** library object */
    LibMuttng* lib;
  public:
    MinClient() {
      /* init library */
      lib = new LibMuttng();
      /* connect MinClient::print() to various message signals */
      connectSignal(lib->displayMessage,this,&MinClient::print);
      connectSignal(lib->displayProgress,this,&MinClient::print);
      connectSignal(lib->displayWarning,this,&MinClient::print);
      connectSignal(lib->displayError,this,&MinClient::print);
    }
    ~MinClient() {
      /* cleanup and delete library object */
      lib->cleanup();
      delete lib;
    }
};

/**
 * @c main().
 * @param argc Counter for command-line arguments.
 * @param argv Command-line arguments.
 * @return 0 on success, 1 otherwise.
 */
int main (int argc, char** argv) {
  Mailbox* folder = NULL;
  MinClient* client = new MinClient();

  if (argc >= 2) {
    if (!(folder = Mailbox::fromURL (argv[1]))) {
      /* error */
      std::cerr << "error" << std::endl;
      delete client;
      return 1;
    } else {
      /* success */
      std::cout << "success" << std::endl;
      delete folder;
      delete client;
    }
  } else {
    std::cout << "Usage: " << argv[0] << " [url]" << std::endl;
  }
  return 0;
};
