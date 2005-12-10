/** @ingroup libmuttng_unit */
/**
 * @file core/test/qp_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: QP unit tests
 */
#ifndef CORE_TEST_QP_TESTS_H
#define CORE_TEST_QP_TESTS_H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * QP unit test
 */
class qp_tests : public suite {
  public:
    qp_tests();
    ~qp_tests();
  private:
    /** @test buffer_encode_qp(). */
    void test_encode();
    /** @test buffer_decode_qp(). */
    void test_decode();
    /**
     * @test buffer_encode_qp().
     * @test buffer_decode_qp().
     */
    void test_both();
};

#endif
