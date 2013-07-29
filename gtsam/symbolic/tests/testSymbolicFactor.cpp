/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file    testSymbolicFactor.cpp
 * @brief   Unit tests for a symbolic IndexFactor
 * @author  Frank Dellaert
 */

#include <CppUnitLite/TestHarness.h>
#include <gtsam/base/TestableAssertions.h>
#include <gtsam/symbolic/SymbolicFactor.h>
#include <gtsam/symbolic/SymbolicConditional.h>
#include <gtsam/symbolic/SymbolicFactorGraph.h>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/make_shared.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace gtsam;
using namespace boost::assign;

/* ************************************************************************* */
#ifdef TRACK_ELIMINATE
TEST(SymbolicFactor, eliminate) {
  vector<Index> keys; keys += 2, 3, 4, 6, 7, 9, 10, 11;
  IndexFactorOrdered actual(keys.begin(), keys.end());
  BayesNetOrdered<IndexConditionalOrdered> fragment = *actual.eliminate(3);

  IndexFactorOrdered expected(keys.begin()+3, keys.end());
  IndexConditionalOrdered::shared_ptr expected0 = IndexConditionalOrdered::FromRange(keys.begin(), keys.end(), 1);
  IndexConditionalOrdered::shared_ptr expected1 = IndexConditionalOrdered::FromRange(keys.begin()+1, keys.end(), 1);
  IndexConditionalOrdered::shared_ptr expected2 = IndexConditionalOrdered::FromRange(keys.begin()+2, keys.end(), 1);

  CHECK(assert_equal(fragment.size(), size_t(3)));
  CHECK(assert_equal(expected, actual));
  BayesNetOrdered<IndexConditionalOrdered>::const_iterator fragmentCond = fragment.begin();
  CHECK(assert_equal(**fragmentCond++, *expected0));
  CHECK(assert_equal(**fragmentCond++, *expected1));
  CHECK(assert_equal(**fragmentCond++, *expected2));
}
#endif
/* ************************************************************************* */
TEST(SymbolicFactor, EliminateSymbolic)
{
  const SymbolicFactorGraph factors = list_of
    (SymbolicFactor(2,4,6))
    (SymbolicFactor(1,2,5))
    (SymbolicFactor(0,3));

  const SymbolicFactor expectedFactor(4,5,6);
  const SymbolicConditional expectedConditional =
    SymbolicConditional::FromKeys(list_of(0)(1)(2)(3)(4)(5)(6), 4);

  SymbolicFactor::shared_ptr actualFactor;
  SymbolicConditional::shared_ptr actualConditional;
  boost::tie(actualConditional, actualFactor) =
    EliminateSymbolic(factors, list_of(0)(1)(2)(3));

  CHECK(assert_equal(expectedConditional, *actualConditional));
  CHECK(assert_equal(expectedFactor, *actualFactor));
}

/* ************************************************************************* */
int main() {
  TestResult tr;
  return TestRegistry::runAllTests(tr);
}
/* ************************************************************************* */
