/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/conn_tests.cpp
 * @author Andreas Krennmair
 * @brief Implementation: Connection unit tests
 */
#include <unit++/unit++.h>

#include "conn_tests.h"

#include "core/buffer.h"
#include "transport/connection.h"

using namespace unitpp;

void conn_tests::test_connectdisconnect() {
  buffer_t buf;
  buffer_init(&buf);
  buffer_add_str(&buf,"www.google.com",-1);
  Connection * conn = new Connection(&buf,80);

  bool return_value;

  return_value = conn->connect();

  assert_eq("connect() to www.google.com:80",return_value,true);

  return_value = conn->disconnect();

  assert_eq("disconnect() from www.google.com:80",return_value,true);

  buffer_free(&buf);
  delete conn;
}

void conn_tests::test_readwrite() {
  buffer_t buf;
  buffer_init(&buf);
  buffer_add_str(&buf,"www.google.com",-1);
  Connection * conn = new Connection(&buf,80);

  bool return_value;

  return_value = conn->connect();

  assert_eq("connect() to www.google.com:80",return_value,true);

  buffer_t rbuf;
  buffer_init(&rbuf);
  buffer_add_str(&rbuf,"GET / HTTP/1.0\r\nHost: www.google.com\r\nUser-Agent: mutt-ng test suite\r\n\r\n",-1);

  assert_eq("doWrite",conn->doWrite(&rbuf),(signed)rbuf.len);

  // TODO: add read test

  return_value = conn->disconnect();

  assert_eq("disconnect() from www.google.com:80",return_value,true);

  buffer_free(&buf);
  delete conn;
}

conn_tests::conn_tests() : suite("conn_tests") {
  add("conn",testcase(this,"test_connectdisconnect",&conn_tests::test_connectdisconnect));
  add("conn",testcase(this,"test_readwrite",&conn_tests::test_readwrite));
}

conn_tests::~conn_tests() { }
