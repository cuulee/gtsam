/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file LinearizedFactor.h
 * @brief A dummy factor that allows a linear factor to act as a nonlinear factor
 * @author Alex Cunningham
 */

#pragma once

#include <vector>
#include <gtsam_unstable/base/dllexport.h>
#include <gtsam/nonlinear/OrderingOrdered.h>
#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/linear/JacobianFactorOrdered.h>
#include <gtsam/linear/HessianFactorOrdered.h>
#include <gtsam/base/blockMatrices.h>

namespace gtsam {

/**
 * A base factor class for the Jacobian and Hessian linearized factors
 */
class GTSAM_UNSTABLE_EXPORT LinearizedGaussianFactor : public NonlinearFactor {
public:
  /** base type */
  typedef NonlinearFactor Base;
  typedef LinearizedGaussianFactor This;

  /** shared pointer for convenience */
  typedef boost::shared_ptr<LinearizedGaussianFactor> shared_ptr;

protected:

  /** linearization points for error calculation */
  Values lin_points_;

public:

  /** default constructor for serialization */
  LinearizedGaussianFactor() {};

  /**
   * @param gaussian:   A jacobian or hessian factor
   * @param ordering:   The full ordering used to linearize this factor
   * @param lin_points: The linearization points for, at least, the variables used by this factor
   */
  LinearizedGaussianFactor(const GaussianFactorOrdered::shared_ptr& gaussian, const OrderingOrdered& ordering, const Values& lin_points);

  virtual ~LinearizedGaussianFactor() {};

  // access functions
  const Values& linearizationPoint() const { return lin_points_; }

private:
  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("LinearizedGaussianFactor",
        boost::serialization::base_object<Base>(*this));
    ar & BOOST_SERIALIZATION_NVP(lin_points_);
  }

};

/**
 * A factor that takes a linear, Jacobian factor and inserts it into
 * a nonlinear graph.
 */
class GTSAM_UNSTABLE_EXPORT LinearizedJacobianFactor : public LinearizedGaussianFactor {

public:
  /** base type */
  typedef LinearizedGaussianFactor Base;
  typedef LinearizedJacobianFactor This;

  /** shared pointer for convenience */
  typedef boost::shared_ptr<LinearizedJacobianFactor> shared_ptr;

  typedef Matrix AbMatrix;
  typedef VerticalBlockView<AbMatrix> BlockAb;
  typedef BlockAb::Block ABlock;
  typedef BlockAb::constBlock constABlock;
  typedef BlockAb::Column BVector;
  typedef BlockAb::constColumn constBVector;

protected:

//  // store components of a jacobian factor
//  typedef std::map<Key, Matrix> KeyMatrixMap;
//  KeyMatrixMap matrices_;
//  Vector b_;

  AbMatrix matrix_; // the full matrix corresponding to the factor
  BlockAb Ab_;      // the block view of the full matrix

public:

  /** default constructor for serialization */
  LinearizedJacobianFactor();

  /**
   * @param jacobian:   A jacobian factor
   * @param ordering:   The ordering used to linearize this factor
   * @param lin_points: The linearization points for, at least, the variables used by this factor
   */
  LinearizedJacobianFactor(const JacobianFactorOrdered::shared_ptr& jacobian,
      const OrderingOrdered& ordering, const Values& lin_points);

  virtual ~LinearizedJacobianFactor() {}

  /// @return a deep copy of this factor
  virtual gtsam::NonlinearFactor::shared_ptr clone() const {
    return boost::static_pointer_cast<gtsam::NonlinearFactor>(
        gtsam::NonlinearFactor::shared_ptr(new This(*this))); }

  // Testable

  /** print function */
  virtual void print(const std::string& s="", const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /** equals function with optional tolerance */
  virtual bool equals(const NonlinearFactor& expected, double tol = 1e-9) const;

  // access functions
  const constBVector b() const { return Ab_.column(size(), 0); }
  const constABlock A() const { return Ab_.range(0, size()); };
  const constABlock A(Key key) const { return Ab_(std::find(begin(), end(), key) - begin()); }

  /** get the dimension of the factor (number of rows on linearization) */
  size_t dim() const { return Ab_.rows(); };

  /** Calculate the error of the factor */
  double error(const Values& c) const;

  /**
   * linearize to a GaussianFactor
   * Reimplemented from NoiseModelFactor to directly copy out the
   * matrices and only update the RHS b with an updated residual
   */
  boost::shared_ptr<GaussianFactorOrdered> linearize(
      const Values& c, const OrderingOrdered& ordering) const;

  /** (A*x-b) */
  Vector error_vector(const Values& c) const;

private:
  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("LinearizedJacobianFactor",
        boost::serialization::base_object<Base>(*this));
    ar & BOOST_SERIALIZATION_NVP(matrix_);
    ar & BOOST_SERIALIZATION_NVP(Ab_);
  }
};




/**
 * A factor that takes a linear, Hessian factor and inserts it into
 * a nonlinear graph.
 */
class GTSAM_UNSTABLE_EXPORT LinearizedHessianFactor : public LinearizedGaussianFactor {

public:
  /** base type */
  typedef LinearizedGaussianFactor Base;
  typedef LinearizedHessianFactor This;

  /** shared pointer for convenience */
  typedef boost::shared_ptr<LinearizedHessianFactor> shared_ptr;

  /** hessian block data types */
  typedef Matrix InfoMatrix; ///< The full augmented Hessian
  typedef SymmetricBlockView<InfoMatrix> BlockInfo; ///< A blockwise view of the Hessian
  typedef BlockInfo::Block Block; ///< A block from the Hessian matrix
  typedef BlockInfo::constBlock constBlock; ///< A block from the Hessian matrix (const version)
  typedef BlockInfo::Column Column; ///< A column containing the linear term h
  typedef BlockInfo::constColumn constColumn; ///< A column containing the linear term h (const version)

protected:

  InfoMatrix matrix_; ///< The full augmented information matrix, s.t. the quadratic error is 0.5*[x -1]'*H*[x -1]
  BlockInfo info_;    ///< The block view of the full information matrix.

public:

  /** default constructor for serialization */
  LinearizedHessianFactor();

  /**
   * Use this constructor with the ordering used to linearize the graph
   * @param hessian:    A hessian factor
   * @param ordering:   The ordering used to linearize this factor
   * @param lin_points: The linearization points for, at least, the variables used by this factor
   */
  LinearizedHessianFactor(const HessianFactorOrdered::shared_ptr& hessian,
      const OrderingOrdered& ordering, const Values& lin_points);

  virtual ~LinearizedHessianFactor() {}

  /// @return a deep copy of this factor
  virtual gtsam::NonlinearFactor::shared_ptr clone() const {
    return boost::static_pointer_cast<gtsam::NonlinearFactor>(
        gtsam::NonlinearFactor::shared_ptr(new This(*this))); }

  // Testable

  /** print function */
  virtual void print(const std::string& s="", const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /** equals function with optional tolerance */
  virtual bool equals(const NonlinearFactor& expected, double tol = 1e-9) const;

  /** Return the constant term \f$ f \f$ as described above
   * @return The constant term \f$ f \f$
   */
  double constantTerm() const { return info_(this->size(), this->size())(0,0); };

  /** Return the part of linear term \f$ g \f$ as described above corresponding to the requested variable.
   * @param j Which block row to get, as an iterator pointing to the slot in this factor.  You can
   * use, for example, begin() + 2 to get the 3rd variable in this factor.
   * @return The linear term \f$ g \f$ */
  constColumn linearTerm(const_iterator j) const { return info_.column(j - this->begin(), this->size(), 0); }

  /** Return the complete linear term \f$ g \f$ as described above.
   * @return The linear term \f$ g \f$ */
  constColumn linearTerm() const { return info_.rangeColumn(0, this->size(), this->size(), 0); };

  /** Return a view of the block at (j1,j2) of the <emph>upper-triangular part</emph> of the
   * squared term \f$ H \f$, no data is copied.  See HessianFactor class documentation
   * above to explain that only the upper-triangular part of the information matrix is stored
   * and returned by this function.
   * @param j1 Which block row to get, as an iterator pointing to the slot in this factor.  You can
   * use, for example, begin() + 2 to get the 3rd variable in this factor.
   * @param j2 Which block column to get, as an iterator pointing to the slot in this factor.  You can
   * use, for example, begin() + 2 to get the 3rd variable in this factor.
   * @return A view of the requested block, not a copy.
   */
  constBlock squaredTerm(const_iterator j1, const_iterator j2) const { return info_(j1-begin(), j2-begin()); }

  /** Return the <emph>upper-triangular part</emph> of the full squared term, as described above.
   * See HessianFactor class documentation above to explain that only the
   * upper-triangular part of the information matrix is stored and returned by this function.
   */
  constBlock squaredTerm() const { return info_.range(0, this->size(), 0, this->size()); }


  /** get the dimension of the factor (number of rows on linearization) */
  size_t dim() const { return matrix_.rows() - 1; };

  /** Calculate the error of the factor */
  double error(const Values& c) const;

  /**
   * linearize to a GaussianFactor
   * Reimplemented from NoiseModelFactor to directly copy out the
   * matrices and only update the RHS b with an updated residual
   */
  boost::shared_ptr<GaussianFactorOrdered> linearize(
      const Values& c, const OrderingOrdered& ordering) const;

private:
  /** Serialization function */
  friend class boost::serialization::access;
  template<class ARCHIVE>
  void serialize(ARCHIVE & ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("LinearizedHessianFactor",
        boost::serialization::base_object<Base>(*this));
    ar & BOOST_SERIALIZATION_NVP(matrix_);
    ar & BOOST_SERIALIZATION_NVP(info_);
  }
};



} // \namespace aspn
