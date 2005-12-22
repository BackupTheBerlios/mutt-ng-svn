/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/url_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: URL unit tests
 */
#include <unit++/unit++.h>

#ifndef LIBMUTTNG_IMAP
/** define IMAP feature by force to have something to test */
#define LIBMUTTNG_IMAP  1
#endif

#include "url_tests.h"

#include "core/buffer.h"
#include "core/str.h"

/** we need this to avoid default initializer for url_t */
#include "util/url.h"

/**
 * Test table of invalid protocols or valid protocols but
 * invalid specs (eg a host for @c file://).
 */
static const char* InvalidProtoTable[] = {
  "mailto:foo@bar", "file://host/path", "file://user@host/path",
  NULL
};

/**
 * Test table for detecting invalid fields.
 */
static const char* InvalidURLTable[] = {
  "imaps://@location/",                 /* empty username */
  "imaps://user:@location/",            /* empty password */
  "imaps:///path",                      /* empty host */
  NULL
};

/**
 * Test table for valid urls.
 */
static struct {
  /** url string to parse */
  const char* urlstr;
  /** assumed result for url_eq(). */
  url_t url;
} ValidURLTable[] = {

  /* imap */
  { "imap://location",                           { NULL,   NULL,     "location", 0,0,   false, "/",   P_IMAP } },
  { "imap://location/",                          { NULL,   NULL,     "location", 0,0,   false, "/",   P_IMAP } },
  { "imap://location/path",                      { NULL,   NULL,     "location", 0,0,   false, "/path", P_IMAP } },
  { "imap://user@location/path",                 { "user", NULL,     "location", 0,0,   false, "/path", P_IMAP } },
  { "imap://user:secret@location/path",          { "user", "secret", "location", 0,0,   false, "/path", P_IMAP } },
  { "imap://user:secret@location:0815/path",     { "user", "secret", "location", 815,0, false, "/path", P_IMAP } },
  { "imap://user@host:secret@location:0815/path",
    { "user@host", "secret", "location", 815, 0, false, "/path", P_IMAP } },
  /* mostly same as above but secure */
  { "imaps://location",                          { NULL,   NULL,     "location", 0,0,   true,  "/",   P_IMAP } },
  { "imaps://location/",                         { NULL,   NULL,     "location", 0,0,   true,  "/",   P_IMAP } },
  { "imaps://location/path",                     { NULL,   NULL,     "location", 0,0,   true,  "/path", P_IMAP } },
  { "imaps://user@location/path",                { "user", NULL,     "location", 0,0,   true,  "/path", P_IMAP } },
  { "imaps://user:secret@location/path",         { "user", "secret", "location", 0,0,   true,  "/path", P_IMAP } },
  { "imaps://user:secret@location:0815/path",    { "user", "secret", "location", 815,0, true,  "/path", P_IMAP } },
  /* raw IPv4 addresses in host part */
  { "imaps://user@1.2.3.4:815/path",             { "user", NULL,     "1.2.3.4",  815,0, true,  "/path", P_IMAP } },
  /* raw IPv6 address for loopback in host part */
  { "imaps://user@[::1]:815/path",               { "user", NULL,     "::1",      815,0, true,  "/path", P_IMAP } },
  /* raw IPv6 address with KAME-style interface embedded for loopback */
  { "imaps://user@[fe80::1%25lo0]:815/path",     { "user", NULL,     "fe80::1%lo0", 815,0, true,  "/path", P_IMAP } },
  /* raw IPv6 address with KAME-style interface embedded for loopback */
  { "imaps://user@[fe80::1%25lo0]/path",         { "user", NULL,     "fe80::1%lo0", 0,0, true,  "/path", P_IMAP } },
  /* file:// is somewhat special */
  { "file:///tmp",                               { NULL,   NULL,     NULL,       0,0,   false, "/tmp", P_FILE } },
  { "file:///",                                  { NULL,   NULL,     NULL,       0,0,   false, "/",    P_FILE } },
  { "file:///dev%2Fn%75ll",                      { NULL,   NULL,     NULL,       0,0,   false, "/dev/null",    P_FILE } },
  { "file:///dev%2fn%75ll",                      { NULL,   NULL,     NULL,       0,0,   false, "/dev/null",    P_FILE } },

  { NULL,               { NULL, NULL, NULL, 0, 0, false, NULL, P_LAST } }
};

using namespace unitpp;

void url_tests::test_invalid_proto() {
  url_t* url = NULL;
  buffer_t error, msg;
  int i = 0;

  buffer_init((&error));
  buffer_init((&msg));

  for (i = 0; InvalidProtoTable[i]; i++) {
    buffer_shrink (&error, 0);
    buffer_shrink (&msg, 0);
    buffer_add_str (&msg, "URL: '", 6);
    buffer_add_str (&msg, InvalidProtoTable[i], -1);
    buffer_add_str (&msg, "' has invalid proto: ", 21);
    url = url_from_string (InvalidProtoTable[i], &error);
    buffer_add_buffer (&msg, &error);
    assert_true (msg.str, url == NULL);
  }

  buffer_free (&error);
  buffer_free (&msg);
}

void url_tests::test_parse() {
  url_t* url = NULL;
  buffer_t error, msg;
  int i = 0;

  buffer_init((&error));
  buffer_init((&msg));

  for (i = 0; ValidURLTable[i].urlstr; i++) {
    buffer_shrink (&error, 0);
    buffer_shrink (&msg, 0);
    buffer_add_str (&msg, "URL '", 5);
    buffer_add_str (&msg, ValidURLTable[i].urlstr, -1);
    buffer_add_str (&msg, "' is valid: ", 12);
    url = url_from_string (ValidURLTable[i].urlstr, &error);
    buffer_add_buffer (&msg, &error);
    assert_eq (msg.str, 1, url && url_eq (url, &ValidURLTable[i].url));
  }

  for (i = 0; InvalidURLTable[i]; i++) {
    buffer_shrink (&error, 0);
    buffer_shrink (&msg, 0);
    buffer_add_str (&msg, "URL: '", 6);
    buffer_add_str (&msg, InvalidURLTable[i], -1);
    buffer_add_str (&msg, "' is invalid: ", 14);
    url = url_from_string (InvalidURLTable[i], &error);
    buffer_add_buffer (&msg, &error);
    assert_true (msg.str, url == NULL);
  }

  buffer_free (&error);
  buffer_free (&msg);
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
  int i = 0;
  /* for all ports of 0, get defaults so url_eq() works */
  for (i = 0; ValidURLTable[i].urlstr; i++)
    if (!ValidURLTable[i].url.port)
      ValidURLTable[i].url.port = url_defport(ValidURLTable[i].url.proto,
                                              ValidURLTable[i].url.secure);
  add("url",testcase(this,"test_url_invalid_proto",
                     &url_tests::test_invalid_proto));
  add("url",testcase(this,"test_url_parse",
                     &url_tests::test_parse));
  add("url",testcase(this,"test_url_decode",
                     &url_tests::test_decode));
}
