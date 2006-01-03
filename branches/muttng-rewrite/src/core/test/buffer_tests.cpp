/** @ingroup core_unit */
/**
 * @file core/test/buffer_tests.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: buffer_t unit tests
 */
#include <unit++/unit++.h>
#include <buffer.h>
#include "buffer_tests.h"
#include "../str.h"

using namespace unitpp;

/** @test  buffer_init() */
void buffer_tests::test_buffer_init() {
  buffer_t * b = new buffer_t;
  buffer_init(b);

  assert_true("buffer pointer NULL",b->str == NULL);
  assert_eq("buffer size zero",0U,b->size);
  assert_eq("buffer string length zero",0U,b->len);

  delete b;
}

/** @test buffer_shrink(). */
void buffer_tests::test_buffer_shrink() {
  buffer_t tmp;
  buffer_init(&tmp);
  size_t i = 0, max = 400;      /* max must be large enough to trigger realloc() */
  size_t l,s;

  buffer_t* x = NULL;
  buffer_shrink(x,0);
  buffer_shrink(x,1000000000);
  assert_true("didn't crash 'till now ;-)",1);

  for (i = 0; i<max; i++)
    buffer_add_ch(&tmp,'a');

  s = tmp.size;
  l = tmp.len;

  buffer_shrink(&tmp,2*max);
  assert_eq("shrinking to larger value failed (still same size)",s,tmp.size);
  assert_eq("shrinking to larger value failed (still same length)",l,tmp.len);

  buffer_shrink(&tmp,max/2);
  assert_eq("shrinking to smaller value worked (still same size)",s,tmp.size);
  assert_eq("shrinking to smaller value worked (half size)",l/2,tmp.len);

  buffer_free(&tmp);
}

/** @test buffer_grow(). */
void buffer_tests::test_buffer_grow() {
  buffer_t tmp;
  buffer_init(&tmp);

  buffer_grow(&tmp,0);
  assert_true("grow(0) has room for at least \\0",tmp.size>0);
  assert_eq("grow(0) is empty string",0,str_ncmp(tmp.str,"\0",1));
  assert_eq("grow(0) stores \\0",'\0',*tmp.str);

  buffer_grow(&tmp,23);
  assert_true("grow(23) has room for 23+\\0",tmp.size>=24);

  buffer_add_str(&tmp,"foobar",6);

  size_t s = tmp.size;
  size_t l = tmp.len;
  buffer_grow(&tmp,12);
  assert_eq("growing to smaller value does nothing (same size)",s,tmp.size);
  assert_eq("growing to smaller value does nothing (same length)",l,tmp.len);

  buffer_free(&tmp);
}

/**
 * @test  buffer_equal1().
 * @todo implement more tests
 */
void buffer_tests::test_buffer_equal1() {
  buffer_t * b = new buffer_t;
  buffer_init(b);

  assert_true("buffer is empty",buffer_equal1(b,"",-1));
  delete b;
}

/** @test  buffer_add_str(). */
void buffer_tests::test_buffer_add_str() {
  buffer_t * b = new buffer_t;
  buffer_init(b);

  buffer_add_str(b,"test",-1);
  assert_true("buffer contains test",buffer_equal1(b,"test",-1));

  buffer_add_str(b,"asdf",2);
  assert_true("buffer contains testas",buffer_equal1(b,"testas",-1));

  buffer_add_str(b,"",-1);
  assert_true("buffer still contains testas",buffer_equal1(b,"testas",-1));

  delete b;
}

/** @test  buffer_add_buffer() */
void buffer_tests::test_buffer_add_buffer() {
  buffer_t * b1 = new buffer_t;
  buffer_t * b2 = new buffer_t;
  buffer_t * b3 = new buffer_t;

  buffer_init(b1);
  buffer_init(b2);
  buffer_init(b3);

  buffer_add_str(b1,"foo",-1);
  buffer_add_str(b2,"bar",-1);

  buffer_add_buffer(b3,b1);
  assert_true("b3 contains foo",buffer_equal2(b1,b3));

  buffer_add_buffer(b1,b2);
  assert_true("b1 contains foobar",buffer_equal1(b1,"foobar",-1));

  buffer_add_buffer(b1,b1);
  assert_true("b1 contains foobarfoobar",buffer_equal1(b1,"foobarfoobar",-1));

  delete b1;
  delete b2;
  delete b3;
}

/** @test  buffer_add_ch() */
void buffer_tests::test_buffer_add_ch() {
  buffer_t * b = new buffer_t;
  buffer_init(b);

  buffer_add_ch(b,'3');
  assert_true("buffer contains '3'",buffer_equal1(b,"3",-1));

  buffer_add_ch(b,' ');
  assert_true("buffer contains '3 '",buffer_equal1(b,"3 ",-1));

  /* that's a tricky one */
  buffer_add_ch(b,'\0');
  buffer_add_ch(b,'x');
  assert_true("buffer contains '3 \\0x'",buffer_equal1(b,"3 \0x",4));

  delete b;
}

/** @test  buffer_add_snum2() */
void buffer_tests::test_buffer_add_snum2() {
  buffer_t * b = new buffer_t;
  buffer_init(b);

  buffer_add_snum2(b,23,-1,10);
  assert_true("buffer contains 23",buffer_equal1(b,"23",-1));

  buffer_shrink(b,0);

  buffer_add_snum2(b,24,4,10);
  assert_true("buffer contains 0024",buffer_equal1(b,"0024",-1));

  buffer_shrink(b,0);

  buffer_add_snum2(b,383833,-1,16);
  assert_true("buffer contains 5db59",buffer_equal1(b,"5db59",-1));

  /**
   * - Test edge case: when converting INT_MIN to binary we have
   *   a sign and the largest number of digits, i.e. verify here
   *   that conv_itoa() doesn't run into buffer overflow
   */
  buffer_shrink(b,0);
  buffer_add_snum2(b,INT_MIN,-1,2);
  assert_true("buffer contains INT_MIN (binary)",b->len == (sizeof(int)*8)+1);

  buffer_shrink(b,0);
  buffer_add_snum2(b,LONG_MIN,-1,2);
  assert_true("buffer contains LONG_MIN (binary)",b->len == (sizeof(long)*8)+1);

  /*
   * we have two's complement, i.e. largest positive is:
   * 01111...1, i.e. leading 0 -> do -1
   */
  buffer_shrink(b,0);
  buffer_add_snum2(b,INT_MAX,-1,2);
  assert_true("buffer contains INT_MAX (binary)",b->len == (sizeof(int)*8)-1);

  buffer_shrink(b,0);
  buffer_add_snum2(b,LONG_MAX,-1,2);
  assert_true("buffer contains LONG_MAX (binary)",b->len == (sizeof(long)*8)-1);

  buffer_shrink(b,0);
  buffer_add_snum2(b,ULONG_MAX,-1,2);
  assert_true("wrong usage: buffer_add_snum2(ULONG_MAX)==-1",buffer_equal1(b,"-1",-1));

  delete b;
}

/** @test  buffer_add_unum2() */
void buffer_tests::test_buffer_add_unum2() {
  buffer_t * b = new buffer_t;
  buffer_init(b);

  buffer_add_unum2(b,23,-1,10);
  assert_true("buffer contains 23",buffer_equal1(b,"23",-1));

  buffer_shrink(b,0);

  buffer_add_unum2(b,24,4,10);
  assert_true("buffer contains 0024",buffer_equal1(b,"0024",-1));

  buffer_shrink(b,0);

  buffer_add_unum2(b,383833,-1,16);
  assert_true("buffer contains 5db59",buffer_equal1(b,"5db59",-1));

  /**
   * - Test edge case: when converting UINT_MAX to binary we have
   *   a sign and the largest number of digits, i.e. verify here
   *   that conv_itoa() doesn't run into buffer overflow
   */
  buffer_shrink(b,0);
  buffer_add_unum2(b,UINT_MAX,-1,2);
  assert_true("buffer contains UINT_MAX (binary)",b->len == (sizeof(unsigned int)*8));

  buffer_shrink(b,0);
  buffer_add_unum2(b,ULONG_MAX,-1,2);
  assert_true("buffer contains ULONG_MAX (binary)",b->len == (sizeof(unsigned long)*8));

  delete b;
}

/** @test  buffer_chomp() */
void buffer_tests::test_buffer_chomp() {
  buffer_t buf;
  buffer_init(&buf);
  unsigned int count;

  buffer_add_str(&buf,"",-1);
  count = buffer_chomp(&buf);
  assert_eq("empty string",true,buffer_equal1(&buf,"",-1));
  assert_eq("empty string, chomp count",0U,count);

  buffer_add_str(&buf,"\r\r\r",-1);
  count = buffer_chomp(&buf);
  assert_eq("empty string with trailing CRs",true,buffer_equal1(&buf,"",-1));
  assert_eq("empty string with trailing CRs, chomp count",3U,count);

  buffer_add_str(&buf,"asdf\n",-1);
  count = buffer_chomp(&buf);
  assert_eq("'asdf\\n' chomped",true,buffer_equal1(&buf,"asdf",-1));
  assert_eq("'asdf\\n' chomped, chomp count",1U,count);

  buffer_shrink(&buf,0);
  buffer_add_str(&buf,"\r\n\r\nqwert\r\n\r\n",-1);
  count = buffer_chomp(&buf);

  assert_eq("'\\r\\n\\r\\nqwert\\r\\n\\r\\n' chomped",true,buffer_equal1(&buf,"\r\n\r\nqwert",-1));
  assert_eq("'\\r\\n\\r\\nqwert\\r\\n\\r\\n' chomped,chomp_count",4U,count);

  buffer_free(&buf);

}

/**
 * Helper for test_buffer_tokenize().
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param in Input string.
 * @param expect Expected output string.
 * @param flags Flags for buffer_extract_token().
 * @param l Length of input buffer_extract_token() is expected to read.
 */
static void tokenize(buffer_t* dst, buffer_t* src,
                     const char* in, const char* expect,
                     int flags, int l=-1) {
  if (l<=0) l=str_len(in);
  buffer_shrink(dst,0);
  buffer_shrink(src,0);
  buffer_add_str(src,in,-1);
  size_t len = buffer_extract_token(dst,src,flags,NULL);
  assert_eq("all characters parsed from source",l,(int)len);
  buffer_t msg;
  buffer_init(&msg);
  buffer_add_ch(&msg,'\'');
  buffer_add_str(&msg,in,-1);
  buffer_add_str(&msg,"' -> '",6);
  buffer_add_str(&msg,expect,-1);
  buffer_add_str(&msg,"': '",4);;
  buffer_add_buffer(&msg,dst);
  buffer_add_ch(&msg,'\'');
  assert_true(msg.str,buffer_equal1(dst,expect,-1));
}

/**
 * @test buffer_extract_token()
 * @todo add more tricky tests playing with flags
 */
void buffer_tests::test_buffer_tokenize() {
  buffer_t dst,src;
  buffer_init(&dst);
  buffer_init(&src);

  /** We test: */

  /** - wether distinction between valid and invalid octal chars works */
  tokenize(&dst,&src,"\\888","888",0);
  tokenize(&dst,&src,"\\66f","66f",0);
  tokenize(&dst,&src,"\\146\\157\\157","foo",0);
  /** - wether distinction between valid and invalid hex chars works */
  tokenize(&dst,&src,"\\x4s","x4s",0);
  tokenize(&dst,&src,"\\x42","B",0);
  tokenize(&dst,&src,"\\x66\\x6f\\x6F","foo",0);
  /** - wether escape sequences are replaced accordingly */
  tokenize(&dst,&src,"\\Ca","\001",0);
  tokenize(&dst,&src,"\\cA","\001",0);
  tokenize(&dst,&src,"\\r\\n\\t\\f\\e\\E","\015\012\011\014\033\033",0);
  /** - whether ^-expansion is only done with M_TOKEN_CONDENSE and works */
  tokenize(&dst,&src,"^^^[^A^0","^^^[^A^0",0);
  tokenize(&dst,&src,"^^^[^A^0","^\033\001^0",M_TOKEN_CONDENSE);
  /** - whether variable expansion and quoting works */
  tokenize(&dst,&src,"$HOME",getenv("HOME"),0);
  tokenize(&dst,&src,"${HOME}",getenv("HOME"),0);
  tokenize(&dst,&src,"'$HOME'","$HOME",0);
  tokenize(&dst,&src,"'${HOME}'","${HOME}",0);
  tokenize(&dst,&src,"a\\$HOME","a$HOME",0);
  /** - wether M_TOKEN_SPACE and quoting do what they're supposed to */
  tokenize(&dst,&src,"a b","a",0,2);
  tokenize(&dst,&src,"a b","a b",M_TOKEN_SPACE);
  tokenize(&dst,&src,"a\\ b","a b",0);
  tokenize(&dst,&src,"\"a b\" \"b a\"","a b b a",M_TOKEN_SPACE);
  /** - whether backtick expansion and quoting works */
  tokenize(&dst,&src,"'f`echo o`o'","f`echo o`o",0);
  tokenize(&dst,&src,"f`echo o`o","foo",0);
  tokenize(&dst,&src,"\"f`echo o`o\"","foo",0);
  tokenize(&dst,&src,"\"`echo tEstSTRinG | tr 'A-Z' 'a-z' | tr 'A-Za-z' 'N-ZA-Mn-za-n'`\"","grfgfgevat",0);
  tokenize(&dst,&src,"\"`( ls /bin/nonexistent >/dev/null 2>&1) && echo yes || echo no`\"","no",0);
  tokenize(&dst,&src,"\"`( ls / >/dev/null 2>&1) && echo yes || echo no`\"","yes",0);
}

/**
 * Callback for buffer_format().
 * @param dst Destination buffer.
 * @param fmt String format.
 * @param c Expando.
 * @return 1 if known expando, 0 otherwise
 */
static int fmt(buffer_t* dst, buffer_t* fmt, unsigned char c) {
  switch (c) {
  case 'a':
    assert_eq("no fmt::len for %a",0U,fmt->len);
    assert_eq("no fmt::size for %a",0U,fmt->size);
    buffer_add_str(dst,"/dev/null",-1);
    break;
  case 'b':
    assert_eq("1 fmt::len for %a",1U,fmt->len);
    assert_eq("1 fmt::size for %a",1U,fmt->size);
    int tmp = *fmt->str-'0';
    buffer_add_snum(dst,0,tmp);
    break;
  default:
    return 0;
  }
  return 1;
}

/** @test buffer_format(). */
void buffer_tests::test_buffer_format() {
  buffer_t src,dst;

  buffer_init(&src);
  buffer_init(&dst);

  buffer_shrink(&src,0); buffer_shrink(&dst,0);
  buffer_add_str(&src,"vim '%a'",-1);
  assert_eq("all fields of buffer_format(%a)",1,buffer_format(&dst,&src,fmt));
  assert_true("buffer_format(%a) produces \"vim '/dev/null'\"",buffer_equal1(&dst,"vim '/dev/null'",-1));

  buffer_shrink(&src,0); buffer_shrink(&dst,0);
  buffer_add_str(&src,"vim '%%a'",-1);
  assert_eq("all fields of buffer_format(%%a)",0,buffer_format(&dst,&src,fmt));
  assert_true("buffer_format(%%a) produces \"vim '%a'\"",buffer_equal1(&dst,"vim '%a'",-1));

  buffer_shrink(&src,0); buffer_shrink(&dst,0);
  buffer_add_str(&src,"vim '%4b'",-1);
  assert_eq("all fields of buffer_format(%4b)",1,buffer_format(&dst,&src,fmt));
  assert_true("buffer_format(%4b) produces \"vim '0000'\"",buffer_equal1(&dst,"vim '0000'",-1));
}

buffer_tests::buffer_tests() : suite("buffer_tests") {
  add("buffer",testcase(this,"test_buffer_init",&buffer_tests::test_buffer_init));
  add("buffer",testcase(this,"test_buffer_shrink",&buffer_tests::test_buffer_shrink));
  add("buffer",testcase(this,"test_buffer_grow",&buffer_tests::test_buffer_grow));
  add("buffer",testcase(this,"test_buffer_equal1",&buffer_tests::test_buffer_equal1));
  add("buffer",testcase(this,"test_buffer_add_str",&buffer_tests::test_buffer_add_str));
  add("buffer",testcase(this,"test_buffer_add_buffer",&buffer_tests::test_buffer_add_buffer));
  add("buffer",testcase(this,"test_buffer_add_ch",&buffer_tests::test_buffer_add_ch));
  add("buffer",testcase(this,"test_buffer_add_snum2",&buffer_tests::test_buffer_add_snum2));
  add("buffer",testcase(this,"test_buffer_add_unum2",&buffer_tests::test_buffer_add_unum2));
  add("buffer",testcase(this,"test_buffer_chomp",&buffer_tests::test_buffer_chomp));
  add("buffer",testcase(this,"test_buffer_tokenize",&buffer_tests::test_buffer_tokenize));
  add("buffer",testcase(this,"test_buffer_format",&buffer_tests::test_buffer_format));
}
