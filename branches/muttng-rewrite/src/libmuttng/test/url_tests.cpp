/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/url_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: URL unit tests
 */
#include <unit++/unit++.h>

#ifndef LIBMUTTNG_IMAP
#define LIBMUTTNG_IMAP  1
#endif

#include "url_tests.h"

#include "core/buffer.h"
#include "core/str.h"

#include "util/url.h"

using namespace unitpp;

void url_tests::test_invalid_proto() {
  url_t* url = NULL;
  buffer_t error;

  buffer_init((&error));

  buffer_shrink (&error, 0);
  url = url_from_string ("mailto:foo@bar", &error);
  assert_true (error.str, url == NULL);

  buffer_shrink (&error, 0);
  url = url_from_string ("file://host/path", &error);
  assert_true (error.str, url == NULL);

  buffer_shrink (&error, 0);
  url = url_from_string ("file://user@host/path", &error);
  assert_true (error.str, url == NULL);

  buffer_free (&error);
}

void url_tests::test_parse() {
  url_t* url = NULL;
  buffer_t error;

  buffer_init((&error));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://location/", &error);
  assert_true ("got host+no path",
               url != NULL && str_eq ("location", url->host) &&
               !url->path);

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://location/path", &error);
  assert_true ("got host+path",
               url != NULL && str_eq ("location", url->host) &&
                              str_eq ("path", url->path));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://user@location/path", &error);
  assert_true ("got user+host+path",
               url != NULL && str_eq ("location", url->host) &&
                              str_eq ("path", url->path) &&
                              str_eq ("user", url->username));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://user:secret@location/path", &error);
  assert_true ("got user+password+host+path",
               url != NULL && str_eq ("location", url->host) &&
                              str_eq ("path", url->path) &&
                              str_eq ("user", url->username) &&
                              str_eq ("secret", url->password));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://user:secret@location:0815/path", &error);
  assert_true ("got user+password+host+port+path",
               url != NULL && str_eq ("location", url->host) &&
                              str_eq ("path", url->path) &&
                              str_eq ("user", url->username) &&
                              str_eq ("secret", url->password) &&
                              url->port == 815);
}

void url_tests::test_decode() {
  url_t* url = NULL;
  buffer_t error;

  buffer_init((&error));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://location/path%2", &error);
  assert_true ("incomplete hex seq. %2 detected",
               url == NULL && strstr (error.str, "%2"));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://location/path%FXfoo", &error);
  assert_true ("invalid hex seq. %FX detected",
               url == NULL && strstr (error.str, "%FXfoo"));

  buffer_shrink (&error, 0);
  url = url_from_string ("imap://location/path%00Foo", &error);
  assert_true ("evil %00 blocked",
               url == NULL && strstr (error.str, "%00Foo"));
}


url_tests::url_tests() : suite("url_tests") {
  add("url",testcase(this,"test_url_invalid_proto",
                     &url_tests::test_invalid_proto));
  add("url",testcase(this,"test_url_parse",
                     &url_tests::test_parse));
  add("url",testcase(this,"test_url_decode",
                     &url_tests::test_decode));
}
