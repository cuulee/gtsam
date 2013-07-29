/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 *  @file   testJacobianFactor.cpp
 *  @brief  Unit tests for Linear Factor
 *  @author Christian Potthast
 *  @author Frank Dellaert
 **/

#include <gtsam/base/TestableAssertions.h>
#include <CppUnitLite/TestHarness.h>

#include <gtsam/linear/JacobianFactor.h>
#include <gtsam/linear/GaussianFactorGraph.h>
#include <gtsam/linear/GaussianConditional.h>
#include <gtsam/linear/VectorValues.h>

#include <boost/assign/list_of.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/map.hpp>

using namespace std;
using namespace gtsam;
using namespace boost::assign;

namespace {
  namespace simple {
    // Terms we'll use
    const vector<pair<Key, Matrix> > terms = list_of<pair<Key,Matrix> >
      (make_pair(5, Matrix3::Identity()))
      (make_pair(10, 2*Matrix3::Identity()))
      (make_pair(15, 3*Matrix3::Identity()));

    // RHS and sigmas
    const Vector b = Vector_(3, 1., 2., 3.);
    const SharedDiagonal noise = noiseModel::Diagonal::Sigmas(Vector_(3, 0.5, 0.5, 0.5));
  }
}

/* ************************************************************************* */
TEST(JacobianFactor, constructors_and_accessors)
{
  using namespace simple;

  // Test for using different numbers of terms
  {
    // b vector only constructor
    JacobianFactor expected(
      boost::make_iterator_range(terms.begin(), terms.begin()), b);
    JacobianFactor actual(b);
    EXPECT(assert_equal(expected, actual));
    EXPECT(assert_equal(b, expected.getb()));
    EXPECT(assert_equal(b, actual.getb()));
    EXPECT(!expected.get_model());
    EXPECT(!actual.get_model());
  }
  {
    // One term constructor
    JacobianFactor expected(
      boost::make_iterator_range(terms.begin(), terms.begin() + 1), b, noise);
    JacobianFactor actual(terms[0].first, terms[0].second, b, noise);
    EXPECT(assert_equal(expected, actual));
    LONGS_EQUAL((long)terms[0].first, (long)actual.keys().back());
    EXPECT(assert_equal(terms[0].second, actual.getA(actual.end() - 1)));
    EXPECT(assert_equal(b, expected.getb()));
    EXPECT(assert_equal(b, actual.getb()));
    EXPECT(noise == expected.get_model());
    EXPECT(noise == actual.get_model());
  }
  {
    // Two term constructor
    JacobianFactor expected(
      boost::make_iterator_range(terms.begin(), terms.begin() + 2), b, noise);
    JacobianFactor actual(terms[0].first, terms[0].second,
      terms[1].first, terms[1].second, b, noise);
    EXPECT(assert_equal(expected, actual));
    LONGS_EQUAL((long)terms[1].first, (long)actual.keys().back());
    EXPECT(assert_equal(terms[1].second, actual.getA(actual.end() - 1)));
    EXPECT(assert_equal(b, expected.getb()));
    EXPECT(assert_equal(b, actual.getb()));
    EXPECT(noise == expected.get_model());
    EXPECT(noise == actual.get_model());
  }
  {
    // Three term constructor
    JacobianFactor expected(
      boost::make_iterator_range(terms.begin(), terms.begin() + 3), b, noise);
    JacobianFactor actual(terms[0].first, terms[0].second,
      terms[1].first, terms[1].second, terms[2].first, terms[2].second, b, noise);
    EXPECT(assert_equal(expected, actual));
    LONGS_EQUAL((long)terms[2].first, (long)actual.keys().back());
    EXPECT(assert_equal(terms[2].second, actual.getA(actual.end() - 1)));
    EXPECT(assert_equal(b, expected.getb()));
    EXPECT(assert_equal(b, actual.getb()));
    EXPECT(noise == expected.get_model());
    EXPECT(noise == actual.get_model());
  }
  {
    // VerticalBlockMatrix constructor
    JacobianFactor expected(
      boost::make_iterator_range(terms.begin(), terms.begin() + 3), b, noise);
    VerticalBlockMatrix blockMatrix(list_of(3)(3)(3)(1), 3);
    blockMatrix(0) = terms[0].second;
    blockMatrix(1) = terms[1].second;
    blockMatrix(2) = terms[2].second;
    blockMatrix(3) = b;
    JacobianFactor actual(terms | boost::adaptors::map_keys, blockMatrix, noise);
    EXPECT(assert_equal(expected, actual));
    LONGS_EQUAL((long)terms[2].first, (long)actual.keys().back());
    EXPECT(assert_equal(terms[2].second, actual.getA(actual.end() - 1)));
    EXPECT(assert_equal(b, expected.getb()));
    EXPECT(assert_equal(b, actual.getb()));
    EXPECT(noise == expected.get_model());
    EXPECT(noise == actual.get_model());
  }
}

/* ************************************************************************* */
//TEST(JabobianFactor, Hessian_conversion) {
//  HessianFactor hessian(0, (Matrix(4,4) <<
//        1.57,        2.695,         -1.1,        -2.35,
//       2.695,      11.3125,        -0.65,      -10.225,
//        -1.1,        -0.65,            1,          0.5,
//       -2.35,      -10.225,          0.5,         9.25).finished(),
//      (Vector(4) << -7.885, -28.5175, 2.75, 25.675).finished(),
//      73.1725);
//
//  JacobianFactor expected(0, (Matrix(2,4) <<
//      1.2530,   2.1508,   -0.8779,  -1.8755,
//           0,   2.5858,    0.4789,  -2.3943).finished(),
//      (Vector(2) << -6.2929, -5.7941).finished(),
//      noiseModel::Unit::Create(2));
//
//  JacobianFactor actual(hessian);
//
//  EXPECT(assert_equal(expected, actual, 1e-3));
//}

/* ************************************************************************* */
TEST( JacobianFactor, construct_from_graph)
{
  GaussianFactorGraph factors;

  double sigma1 = 0.1;
  Matrix A11 = Matrix::Identity(2,2);
  Vector b1(2); b1 << 2, -1;
  factors.add(JacobianFactor(10, A11, b1, noiseModel::Isotropic::Sigma(2, sigma1)));

  double sigma2 = 0.5;
  Matrix A21 = -2 * Matrix::Identity(2,2);
  Matrix A22 = 3 * Matrix::Identity(2,2);
  Vector b2(2); b2 << 4, -5;
  factors.add(JacobianFactor(10, A21, 8, A22, b2, noiseModel::Isotropic::Sigma(2, sigma2)));

  double sigma3 = 1.0;
  Matrix A32 = -4 * Matrix::Identity(2,2);
  Matrix A33 = 5 * Matrix::Identity(2,2);
  Vector b3(2); b3 << 3, -6;
  factors.add(JacobianFactor(8, A32, 12, A33, b3, noiseModel::Isotropic::Sigma(2, sigma3)));

  Matrix A1(6,2); A1 << A11, A21, Matrix::Zero(2,2);
  Matrix A2(6,2); A2 << Matrix::Zero(2,2), A22, A32;
  Matrix A3(6,2); A3 << Matrix::Zero(4,2), A33;
  Vector b(6); b << b1, b2, b3;
  Vector sigmas(6); sigmas << sigma1, sigma1, sigma2, sigma2, sigma3, sigma3;
  JacobianFactor expected(10, A1, 8, A2, 12, A3, b, noiseModel::Diagonal::Sigmas(sigmas));

  // The ordering here specifies the order in which the variables will appear in the combined factor
  JacobianFactor actual(factors, Ordering(list_of(10)(8)(12)));

  EXPECT(assert_equal(expected, actual));
}

/* ************************************************************************* */
TEST(JacobianFactor, error)
{
  JacobianFactor factor(simple::terms, simple::b, simple::noise);

  VectorValues values;
  values.insert(5, Vector::Constant(3, 1.0));
  values.insert(10, Vector::Constant(3, 0.5));
  values.insert(15, Vector::Constant(3, 1.0/3.0));

  Vector expected_unwhitened(3); expected_unwhitened << 2.0, 1.0, 0.0;
  Vector actual_unwhitened = factor.unweighted_error(values);
  EXPECT(assert_equal(expected_unwhitened, actual_unwhitened));

  Vector expected_whitened(3); expected_whitened << 4.0, 2.0, 0.0;
  Vector actual_whitened = factor.error_vector(values);
  EXPECT(assert_equal(expected_whitened, actual_whitened));

  double expected_error = 0.5 * expected_whitened.squaredNorm();
  double actual_error = factor.error(values);
  DOUBLES_EQUAL(expected_error, actual_error, 1e-10);
}

/* ************************************************************************* */
TEST(JacobianFactor, matrices)
{
  JacobianFactor factor(simple::terms, simple::b, simple::noise);

  Matrix jacobianExpected(3, 9);
  jacobianExpected << simple::terms[0].second, simple::terms[1].second, simple::terms[2].second;
  Vector rhsExpected = simple::b;
  Matrix augmentedJacobianExpected(3, 10);
  augmentedJacobianExpected << jacobianExpected, rhsExpected;

  Matrix augmentedHessianExpected =
    augmentedJacobianExpected.transpose() * simple::noise->R().transpose()
    * simple::noise->R() * augmentedJacobianExpected;

  // Hessian
  EXPECT(assert_equal(Matrix(augmentedHessianExpected.topLeftCorner(9,9)), factor.information()));
  EXPECT(assert_equal(augmentedHessianExpected, factor.augmentedInformation()));

  // Whitened Jacobian
  EXPECT(assert_equal(simple::noise->R() * jacobianExpected, factor.jacobian().first));
  EXPECT(assert_equal(simple::noise->R() * rhsExpected, factor.jacobian().second));
  EXPECT(assert_equal(simple::noise->R() * augmentedJacobianExpected, factor.augmentedJacobian()));

  // Unwhitened Jacobian
  EXPECT(assert_equal(jacobianExpected, factor.jacobian(false).first));
  EXPECT(assert_equal(rhsExpected, factor.jacobian(false).second));
  EXPECT(assert_equal(augmentedJacobianExpected, factor.augmentedJacobian(false)));
}

/* ************************************************************************* */
TEST(JacobianFactor, operators )
{
  SharedDiagonal  sigma0_1 = noiseModel::Isotropic::Sigma(2,0.1);

  Matrix I = eye(2);
  Vector b = Vector_(2,0.2,-0.1);
  JacobianFactor lf(1, -I, 2, I, b, sigma0_1);

  VectorValues c;
  c.insert(1, Vector_(2,10.,20.));
  c.insert(2, Vector_(2,30.,60.));

  // test A*x
  Vector expectedE = Vector_(2,200.,400.);
  Vector actualE = lf * c;
  EXPECT(assert_equal(expectedE, actualE));

  // test A^e
  VectorValues expectedX;
  expectedX.insert(1, Vector_(2,-2000.,-4000.));
  expectedX.insert(2, Vector_(2, 2000., 4000.));
  VectorValues actualX = VectorValues::Zero(expectedX);
  lf.transposeMultiplyAdd(1.0, actualE, actualX);
  EXPECT(assert_equal(expectedX, actualX));
}

/* ************************************************************************* */
TEST(JacobianFactor, default_error )
{
  JacobianFactor f;
  double actual = f.error(VectorValues());
  DOUBLES_EQUAL(0.0, actual, 1e-15);
}

//* ************************************************************************* */
TEST(JacobianFactor, empty )
{
  // create an empty factor
  JacobianFactor f;
  EXPECT(f.empty());
}

/* ************************************************************************* */
TEST(JacobianFactor, eliminate)
{
  Matrix A01 = Matrix_(3,3,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0);
  Vector b0 = Vector_(3, 1.5, 1.5, 1.5);
  Vector s0 = Vector_(3, 1.6, 1.6, 1.6);

  Matrix A10 = Matrix_(3,3,
    2.0, 0.0, 0.0,
    0.0, 2.0, 0.0,
    0.0, 0.0, 2.0);
  Matrix A11 = Matrix_(3,3,
    -2.0, 0.0, 0.0,
    0.0, -2.0, 0.0,
    0.0, 0.0, -2.0);
  Vector b1 = Vector_(3, 2.5, 2.5, 2.5);
  Vector s1 = Vector_(3, 2.6, 2.6, 2.6);

  Matrix A21 = Matrix_(3,3,
    3.0, 0.0, 0.0,
    0.0, 3.0, 0.0,
    0.0, 0.0, 3.0);
  Vector b2 = Vector_(3, 3.5, 3.5, 3.5);
  Vector s2 = Vector_(3, 3.6, 3.6, 3.6);

  GaussianFactorGraph gfg;
  gfg.add(1, A01, b0, noiseModel::Diagonal::Sigmas(s0, true));
  gfg.add(0, A10, 1, A11, b1, noiseModel::Diagonal::Sigmas(s1, true));
  gfg.add(1, A21, b2, noiseModel::Diagonal::Sigmas(s2, true));

  Matrix zero3x3 = zeros(3,3);
  Matrix A0 = gtsam::stack(3, &A10, &zero3x3, &zero3x3);
  Matrix A1 = gtsam::stack(3, &A11, &A01, &A21);
  Vector b = gtsam::concatVectors(3, &b1, &b0, &b2);
  Vector sigmas = gtsam::concatVectors(3, &s1, &s0, &s2);

  JacobianFactor combinedFactor(0, A0, 1, A1, b, noiseModel::Diagonal::Sigmas(sigmas, true));
  GaussianFactorGraph::EliminationResult expected = combinedFactor.eliminate(list_of(0));
  JacobianFactor::shared_ptr expectedJacobian = boost::dynamic_pointer_cast<
    JacobianFactor>(expected.second);

  GaussianFactorGraph::EliminationResult actual = EliminateQR(gfg, list_of(0));
  JacobianFactor::shared_ptr actualJacobian = boost::dynamic_pointer_cast<
    JacobianFactor>(actual.second);

  EXPECT(assert_equal(*expected.first, *actual.first));
  EXPECT(assert_equal(*expectedJacobian, *actualJacobian));
}

/* ************************************************************************* */
TEST(JacobianFactor, eliminate2 )
{
  // sigmas
  double sigma1 = 0.2;
  double sigma2 = 0.1;
  Vector sigmas = Vector_(4, sigma1, sigma1, sigma2, sigma2);

  // the combined linear factor
  Matrix Ax2 = Matrix_(4,2,
    // x2
    -1., 0.,
    +0.,-1.,
    1., 0.,
    +0.,1.
    );

  Matrix Al1x1 = Matrix_(4,4,
    // l1   x1
    1., 0., 0.00,  0., // f4
    0., 1., 0.00,  0., // f4
    0., 0., -1.,  0., // f2
    0., 0., 0.00,-1.  // f2
    );

  // the RHS
  Vector b2(4);
  b2(0) = -0.2;
  b2(1) =  0.3;
  b2(2) =  0.2;
  b2(3) = -0.1;

  vector<pair<Index, Matrix> > meas;
  meas.push_back(make_pair(2, Ax2));
  meas.push_back(make_pair(11, Al1x1));
  JacobianFactor combined(meas, b2, noiseModel::Diagonal::Sigmas(sigmas));

  // eliminate the combined factor
  pair<GaussianConditional::shared_ptr, JacobianFactor::shared_ptr>
    actual = combined.eliminate(Ordering(list_of(2)));

  // create expected Conditional Gaussian
  double oldSigma = 0.0894427; // from when R was made unit
  Matrix R11 = Matrix_(2,2,
    1.00,  0.00,
    0.00,  1.00
    )/oldSigma;
  Matrix S12 = Matrix_(2,4,
    -0.20, 0.00,-0.80, 0.00,
    +0.00,-0.20,+0.00,-0.80
    )/oldSigma;
  Vector d = Vector_(2,0.2,-0.14)/oldSigma;
  GaussianConditional expectedCG(2, d, R11, 11, S12);

  EXPECT(assert_equal(expectedCG, *actual.first, 1e-4));

  // the expected linear factor
  double sigma = 0.2236;
  Matrix Bl1x1 = Matrix_(2,4,
    // l1          x1
    1.00, 0.00, -1.00,  0.00,
    0.00, 1.00, +0.00, -1.00
    )/sigma;
  Vector b1 = Vector_(2, 0.0, 0.894427);
  JacobianFactor expectedLF(11, Bl1x1, b1);
  EXPECT(assert_equal(expectedLF, *actual.second,1e-3));
}

/* ************************************************************************* */
TEST(JacobianFactor, EliminateQROrdered)
{
  // Augmented Ab test case for whole factor graph
  Matrix Ab = Matrix_(14,11,
    4.,     0.,     1.,     4.,     1.,     0.,     3.,     6.,     8.,     8.,     1.,
    9.,     2.,     0.,     1.,     6.,     3.,     9.,     6.,     6.,     9.,     4.,
    5.,     3.,     7.,     9.,     5.,     5.,     9.,     1.,     3.,     7.,     0.,
    5.,     6.,     5.,     7.,     9.,     4.,     0.,     1.,     1.,     3.,     5.,
    0.,     0.,     4.,     5.,     6.,     6.,     7.,     9.,     4.,     5.,     4.,
    0.,     0.,     9.,     4.,     8.,     6.,     2.,     1.,     4.,     1.,     6.,
    0.,     0.,     6.,     0.,     4.,     2.,     4.,     0.,     1.,     9.,     6.,
    0.,     0.,     6.,     6.,     4.,     4.,     5.,     5.,     5.,     8.,     6.,
    0.,     0.,     0.,     0.,     8.,     0.,     9.,     8.,     2.,     8.,     0.,
    0.,     0.,     0.,     0.,     0.,     9.,     4.,     6.,     3.,     2.,     0.,
    0.,     0.,     0.,     0.,     1.,     1.,     9.,     1.,     5.,     5.,     3.,
    0.,     0.,     0.,     0.,     1.,     1.,     3.,     3.,     2.,     0.,     5.,
    0.,     0.,     0.,     0.,     0.,     0.,     0.,     0.,     2.,     4.,     6.,
    0.,     0.,     0.,     0.,     0.,     0.,     0.,     0.,     6.,     3.,     4.);

  // Create factor graph
  const SharedDiagonal sig_4D = noiseModel::Isotropic::Sigma(4, 0.5);
  const SharedDiagonal sig_2D = noiseModel::Isotropic::Sigma(2, 0.5);
  GaussianFactorGraph factors = list_of
    (JacobianFactor(list_of(3)(5)(7)(9)(11), VerticalBlockMatrix(list_of(2)(2)(2)(2)(2)(1), Ab.block(0, 0, 4, 11)), sig_4D))
    (JacobianFactor(list_of(5)(7)(9)(11), VerticalBlockMatrix(list_of(2)(2)(2)(2)(1), Ab.block(4, 2, 4, 9)), sig_4D))
    (JacobianFactor(list_of(7)(9)(11), VerticalBlockMatrix(list_of(2)(2)(2)(1), Ab.block(8, 4, 4, 7)), sig_4D))
    (JacobianFactor(list_of(11), VerticalBlockMatrix(list_of(2)(1), Ab.block(12, 8, 2, 3)), sig_2D));

  // extract the dense matrix for the graph
  Matrix actualDense = factors.augmentedJacobian();
  EXPECT(assert_equal(2.0 * Ab, actualDense));

  // Expected augmented matrix, both GaussianConditional (first 6 rows) and remaining factor (next 4 rows)
  Matrix R = 2.0*Matrix_(11,11,
    -12.1244,  -5.1962,  -5.2786,  -8.6603, -10.5573,  -5.9385, -11.3820,  -7.2581,  -8.7427, -13.4440,  -5.3611,
    0.,   4.6904,   5.0254,   5.5432,   5.5737,   3.0153,  -3.0153,  -3.5635,  -3.9290,  -2.7412,   2.1625,
    0.,       0., -13.8160,  -8.7166, -10.2245,  -8.8666,  -8.7632,  -5.2544,  -6.9192, -10.5537,  -9.3250,
    0.,       0.,       0.,   6.5033,  -1.1453,   1.3179,   2.5768,   5.5503,   3.6524,   1.3491,  -2.5676,
    0.,       0.,       0.,       0.,  -9.6242,  -2.1148,  -9.3509, -10.5846,  -3.5366,  -6.8561,  -3.2277,
    0.,       0.,       0.,       0.,       0.,   9.7887,   4.3551,   5.7572,   2.7876,   0.1611,   1.1769,
    0.,       0.,       0.,       0.,       0.,       0., -11.1139,  -0.6521,  -2.1943,  -7.5529,  -0.9081,
    0.,       0.,       0.,       0.,       0.,       0.,       0.,  -4.6479,  -1.9367,  -6.5170,  -3.7685,
    0.,       0.,       0.,       0.,       0.,       0.,       0.,       0.,   8.2503,   3.3757,   6.8476,
    0.,       0.,       0.,       0.,       0.,       0.,       0.,       0.,       0.,  -5.7095,  -0.0090,
    0.,       0.,       0.,       0.,       0.,       0.,       0.,       0.,       0.,       0.,  -7.1635);

  GaussianConditional expectedFragment(
    list_of(3)(5)(7)(9)(11), 3, VerticalBlockMatrix(list_of(2)(2)(2)(2)(2)(1), R));

  // Eliminate (3 frontal variables, 6 scalar columns) using QR !!!!
  GaussianFactorGraph::EliminationResult actual = EliminateQR(factors, list_of(3)(5)(7));
  const JacobianFactor &actualJF = dynamic_cast<const JacobianFactor&>(*actual.second);

  EXPECT(assert_equal(expectedFragment, *actual.first, 0.001));
  EXPECT(assert_equal(size_t(2), actualJF.keys().size()));
  EXPECT(assert_equal(Key(9), actualJF.keys()[0]));
  EXPECT(assert_equal(Key(11), actualJF.keys()[1]));
  EXPECT(assert_equal(Matrix(R.block(6, 6, 4, 2)), actualJF.getA(actualJF.begin()), 0.001));
  EXPECT(assert_equal(Matrix(R.block(6, 8, 4, 2)), actualJF.getA(actualJF.begin()+1), 0.001));
  EXPECT(assert_equal(Vector(R.col(10).segment(6, 4)), actualJF.getb(), 0.001));
  EXPECT(!actualJF.get_model());

  // Eliminate (3 frontal variables, 6 scalar columns) using Cholesky !!!!
  // TODO: HessianFactor
  //GaussianBayesNet actualFragment_Chol = *actualFactor_Chol.eliminate(3, JacobianFactor::SOLVE_CHOLESKY);
  //EXPECT(assert_equal(expectedFragment, actualFragment_Chol, 0.001));
  //EXPECT(assert_equal(size_t(2), actualFactor_Chol.keys().size()));
  //EXPECT(assert_equal(Index(9), actualFactor_Chol.keys()[0]));
  //EXPECT(assert_equal(Index(11), actualFactor_Chol.keys()[1]));
  //EXPECT(assert_equal(Ae1, actualFactor_Chol.getA(actualFactor_Chol.begin()), 0.001)); ////
  //EXPECT(linear_dependent(Ae2, actualFactor_Chol.getA(actualFactor_Chol.begin()+1), 0.001));
  //EXPECT(assert_equal(be, actualFactor_Chol.getb(), 0.001)); ////
  //EXPECT(assert_equal(ones(4), actualFactor_Chol.get_sigmas(), 0.001));
}

/* ************************************************************************* */
TEST ( JacobianFactor, constraint_eliminate1 )
{
  // construct a linear constraint
  Vector v(2); v(0)=1.2; v(1)=3.4;
  JacobianFactor lc(1, eye(2), v, noiseModel::Constrained::All(2));

  // eliminate it
  pair<GaussianConditional::shared_ptr, JacobianFactor::shared_ptr>
    actual = lc.eliminate(list_of(1));

  // verify linear factor
  EXPECT(actual.second->size() == 0);

  // verify conditional Gaussian
  Vector sigmas = Vector_(2, 0.0, 0.0);
  GaussianConditional expCG(1, v, eye(2), noiseModel::Diagonal::Sigmas(sigmas));
  EXPECT(assert_equal(expCG, *actual.first));
}

/* ************************************************************************* */
TEST ( JacobianFactor, constraint_eliminate2 )
{
  // Construct a linear constraint
  // RHS
  Vector b(2); b(0)=3.0; b(1)=4.0;

  // A1 - invertible
  Matrix A1(2,2);
  A1(0,0) = 1.0 ; A1(0,1) = 2.0;
  A1(1,0) = 2.0 ; A1(1,1) = 1.0;

  // A2 - not invertible
  Matrix A2(2,2);
  A2(0,0) = 1.0 ; A2(0,1) = 2.0;
  A2(1,0) = 2.0 ; A2(1,1) = 4.0;

  JacobianFactor lc(1, A1, 2, A2, b, noiseModel::Constrained::All(2));

  // eliminate x and verify results
  pair<GaussianConditional::shared_ptr, JacobianFactor::shared_ptr>
    actual = lc.eliminate(list_of(1));

  // LF should be empty
  // It's tricky to create Eigen matrices that are only zero along one dimension
  Matrix m(1,2);
  Matrix Aempty = m.topRows(0);
  Vector bempty = m.block(0,0,0,1);
  JacobianFactor expectedLF(2, Aempty, bempty, noiseModel::Constrained::All(0));
  EXPECT(assert_equal(expectedLF, *actual.second));

  // verify CG
  Matrix R = Matrix_(2, 2,
      1.0,    2.0,
      0.0,    1.0);
  Matrix S = Matrix_(2,2,
      1.0,    2.0,
      0.0,    0.0);
  Vector d = Vector_(2, 3.0, 0.6666);
  Vector sigmas = Vector_(2, 0.0, 0.0);
  GaussianConditional expectedCG(1, d, R, 2, S, noiseModel::Diagonal::Sigmas(sigmas));
  EXPECT(assert_equal(expectedCG, *actual.first, 1e-4));
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */
