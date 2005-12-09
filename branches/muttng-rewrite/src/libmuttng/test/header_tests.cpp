/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/header_tests.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: Header unit tests
 */
#include "message/simple_header.h"
#include "header_tests.h"

header_tests::header_tests() : suite("header_tests") {
  add("header",testcase(this,"test_constructors",&header_tests::test_constructors));
  add("header",testcase(this,"test_equalsname",&header_tests::test_equalsname));
  add("header",testcase(this,"test_serialization",&header_tests::test_serialization));
  add("header",testcase(this,"test_parse",&header_tests::test_parse));
}

header_tests::~header_tests() { }

void header_tests::test_constructors() {
  Header * h = new SimpleHeader();
  assert_eq("empty header name",true,buffer_equal1(h->getName(),"",-1));
  assert_eq("empty header body",true,buffer_equal1(h->getBody(),"",-1));
  delete h;

  h = new SimpleHeader("foobar","quux");
  assert_eq("header name equals foobar",true,buffer_equal1(h->getName(),"foobar",-1));
  assert_eq("header body equals quux",true,buffer_equal1(h->getBody(),"quux",-1));
  delete h;

  h = new SimpleHeader("foo",0);
  assert_eq("header name equals foo",true,buffer_equal1(h->getName(),"foo",-1));
  assert_eq("header body empty",true,buffer_equal1(h->getBody(),"",-1));
  delete h;

  buffer_t a, b;
  buffer_init(&a);
  buffer_init(&b);
  buffer_add_str(&a,"From",-1);
  buffer_add_str(&b,"Some Person <someperson@example.com>",-1);

  h = new SimpleHeader(&a,&b);
  assert_eq("header name equals From",true,buffer_equal1(h->getName(),"From",-1));
  assert_eq("header body equals name + mail address",true,buffer_equal1(h->getBody(),"Some Person <someperson@example.com>",-1));
  delete h;

  buffer_free(&a);
  buffer_free(&b);
}

void header_tests::test_equalsname() {
  Header * h = new SimpleHeader("From","foobar");
  assert_eq("header name equals 'From'",true,h->equalsName("From"));
  assert_eq("header name equals 'from'",true,h->equalsName("from"));
  assert_eq("header name equals 'FROM'",true,h->equalsName("FROM"));
  assert_eq("header name equals 'fROm'",true,h->equalsName("fROm"));

  buffer_t a;
  buffer_init(&a);
  buffer_add_str(&a,"From",-1);

  assert_eq("header name equals buffer_t 'From'",true,h->equalsName(&a));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"from",-1);

  assert_eq("header name equals buffer_t 'from'",true,h->equalsName(&a));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"FROM",-1);

  assert_eq("header name equals buffer_t 'FROM'",true,h->equalsName(&a));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"fROm",-1);

  assert_eq("header name equals buffer_t 'fROm'",true,h->equalsName(&a));

  buffer_free(&a);

  delete h;
}

void header_tests::test_serialization() {
  buffer_t a;
  Header * h1;
  Header * h2;

  buffer_init(&a);

  h1 = new SimpleHeader("From","test1@test1.com");
  h2 = new SimpleHeader("To","test2@test2.com");

  h1->serialize(&a);
  buffer_add_str(&a,"\r\n",-1);
  h2->serialize(&a);
  buffer_add_str(&a,"\r\n",-1);

  assert_eq("serialization of two headers",true,buffer_equal1(&a,"From: test1@test1.com\r\nTo: test2@test2.com\r\n",-1));

  delete h1;
  delete h2;

  h1 = new SimpleHeader("","");

  buffer_shrink(&a,0);
  h1->serialize(&a);
  assert_eq("serialization of an empty header",true,buffer_equal1(&a,": ",-1));

  delete h1;
}

void header_tests::test_parse() {
  buffer_t a;
  buffer_init(&a);
  buffer_add_str(&a,"From: asdf",-1);

  Header * h = new SimpleHeader();
  h->parseLine(&a);

  assert_eq("parse simple header, name",true,buffer_equal1(h->getName(),"From",-1));
  assert_eq("parse simple header, body",true,buffer_equal1(h->getBody(),"asdf",-1));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"To: qwertz\r\n",-1);

  h->parseLine(&a);

  assert_eq("parse simple header w/ NL, name",true,buffer_equal1(h->getName(),"To",-1));
  assert_eq("parse simple header w/ NL, body",true,buffer_equal1(h->getBody(),"qwertz",-1));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"Received: from [000.111.222.333] (helo=foobar.asdfqwerzuui.de)\r\n\tby mx.asdfqwerzuui.de (node=mx0000) with ESMTP (Blafasel),\r\n\tid 0MKsEO-1EjZkM08Kv-0003tP for Krennmair@webdynamite.com; Tue, 06 Dec 2005 11:08:30 +0100\r\n",-1);

  h->parseLine(&a);
  
  assert_eq("parse multi-line header, name",true,buffer_equal1(h->getName(),"Received",-1));
  assert_eq("parse multi-line header, body",true,buffer_equal1(h->getBody(),"from [000.111.222.333] (helo=foobar.asdfqwerzuui.de)\r\n\tby mx.asdfqwerzuui.de (node=mx0000) with ESMTP (Blafasel),\r\n\tid 0MKsEO-1EjZkM08Kv-0003tP for Krennmair@webdynamite.com; Tue, 06 Dec 2005 11:08:30 +0100",-1));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"Bla:fasel",-1);
  
  h->parseLine(&a);

  assert_eq("parse header with no space, name",true,buffer_equal1(h->getName(),"Bla",-1));
  assert_eq("parse header with no space, body",true,buffer_equal1(h->getBody(),"fasel",-1));

  buffer_shrink(&a,0);
  buffer_add_str(&a,":",-1);
  h->parseLine(&a);
  assert_eq("parse empty header",true,buffer_equal1(h->getName(),"",-1) && buffer_equal1(h->getBody(),"",-1));

  buffer_shrink(&a,0);
  buffer_add_str(&a,": ",-1);
  h->parseLine(&a);
  assert_eq("parse empty header as produced by serialization function",true,buffer_equal1(h->getName(),"",-1) && buffer_equal1(h->getBody(),"",-1));

  buffer_shrink(&a,0);
  buffer_add_str(&a,"foo:          bla   \n",-1);
  h->parseLine(&a);
  assert_eq("parse header with many spaces, name",true,buffer_equal1(h->getName(),"foo",-1));
  assert_eq("parse header with many spaces, body",true,buffer_equal1(h->getBody(),"bla   ",-1));

  buffer_free(&a);
  delete h;
}
