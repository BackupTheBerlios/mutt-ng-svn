/** @ingroup core_unit */
/**
 * @file core/test/buffer_tests.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: buffer_t unit tests
 */
#include <unit++/unit++.h>
#include <buffer.h>
#include "buffer_tests.h"

using namespace unitpp;

/** test for buffer_init() */
void buffer_tests::test_buffer_init() {
	buffer_t * b = new buffer_t;
	buffer_init(b);

	assert_true("buffer pointer NULL",b->str == NULL);
	assert_eq("buffer size zero",0U,b->size);
	assert_eq("buffer string length zero",0U,b->len);

	delete b;
}

/**
 * test for buffer_equal1().
 * @todo implement more tests
 */
void buffer_tests::test_buffer_equal1() {
	buffer_t * b = new buffer_t;
	buffer_init(b);

	assert_true("buffer is empty",buffer_equal1(b,"",-1));
	delete b;
}

/** test for buffer_add_str(). */
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

/** test for buffer_add_buffer() */
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

/** test for buffer_add_ch() */
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

/** test for buffer_add_num2() */
void buffer_tests::test_buffer_add_num2() {
	buffer_t * b = new buffer_t;
	buffer_init(b);

	buffer_add_num2(b,23,-1,10);
	assert_true("buffer contains 23",buffer_equal1(b,"23",-1));

	buffer_shrink(b,0);

	buffer_add_num2(b,24,4,10);
	assert_true("buffer contains 0024",buffer_equal1(b,"0024",-1));

	buffer_shrink(b,0);

	buffer_add_num2(b,383833,-1,16);
	assert_true("buffer contains 5db59",buffer_equal1(b,"5db59",-1));

	/**
	 * - Test edge case: when converting INT_MIN to binary we have
	 *   a sign and the largest number of digits, i.e. verify here
	 *   that conv_itoa() doesn't run into buffer overflow
	 */
	buffer_shrink(b,0);
	buffer_add_num2(b,INT_MIN,-1,2);
	assert_true("buffer contains INT_MIN (binary)",b->len == (sizeof(int)*8)+1);

	buffer_shrink(b,0);
	buffer_add_num2(b,INT_MAX,-1,2);
	/*
	 * we have two's complement, i.e. largest positive is:
	 * 01111...1, i.e. leading 0 -> do -1
	 */
	assert_true("buffer contains INT_MAX (binary)",b->len == (sizeof(int)*8)-1);

	delete b;
}

buffer_tests::buffer_tests() : suite("buffer_tests") {
	add("buffer",testcase(this,"test_buffer_init",&buffer_tests::test_buffer_init));
	add("buffer",testcase(this,"test_buffer_equal1",&buffer_tests::test_buffer_equal1));
	add("buffer",testcase(this,"test_buffer_add_str",&buffer_tests::test_buffer_add_str));
	add("buffer",testcase(this,"test_buffer_add_buffer",&buffer_tests::test_buffer_add_buffer));
	add("buffer",testcase(this,"test_buffer_add_ch",&buffer_tests::test_buffer_add_ch));
	add("buffer",testcase(this,"test_buffer_add_num2",&buffer_tests::test_buffer_add_num2));
}
