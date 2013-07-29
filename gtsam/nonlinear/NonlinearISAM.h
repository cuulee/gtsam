/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file NonlinearISAM.h
 * @date Jan 19, 2010
 * @author Viorela Ila and Richard Roberts
 */

#pragma once

#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/linear/GaussianISAMOrdered.h>

namespace gtsam {
/**
 * Wrapper class to manage ISAM in a nonlinear context
 */
class GTSAM_EXPORT NonlinearISAM {
protected:

  /** The internal iSAM object */
  gtsam::GaussianISAMOrdered isam_;

  /** The current linearization point */
  Values linPoint_;

  /** The ordering */
  gtsam::OrderingOrdered ordering_;

  /** The original factors, used when relinearizing */
  NonlinearFactorGraph factors_;

  /** The reordering interval and counter */
  int reorderInterval_;
  int reorderCounter_;

public:

  /// @name Standard Constructors
  /// @{

  /**
   * Periodically reorder and relinearize
   * @param reorderInterval is the number of updates between reorderings,
   *   0 never reorders (and is dangerous for memory consumption)
   *  1 (default) reorders every time, in worse case is batch every update
   *  typical values are 50 or 100
   */
  NonlinearISAM(int reorderInterval = 1) : reorderInterval_(reorderInterval), reorderCounter_(0) {}

  /// @}
  /// @name Standard Interface
  /// @{

  /** Return the current solution estimate */
  Values estimate() const;

  /** find the marginal covariance for a single variable */
  Matrix marginalCovariance(Key key) const;

  // access

  /** access the underlying bayes tree */
  const GaussianISAMOrdered& bayesTree() const { return isam_; }

  /** Return the current linearization point */
  const Values& getLinearizationPoint() const { return linPoint_; }

  /** Get the ordering */
  const gtsam::OrderingOrdered& getOrdering() const { return ordering_; }

  /** get underlying nonlinear graph */
  const NonlinearFactorGraph& getFactorsUnsafe() const { return factors_; }

    /** get counters */
  int reorderInterval() const { return reorderInterval_; }  ///<TODO: comment
  int reorderCounter() const { return reorderCounter_; }    ///<TODO: comment

  /** prints out all contents of the system */
  void print(const std::string& s="", const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /** prints out clique statistics */
  void printStats() const;

  /** saves the Tree to a text file in GraphViz format */
  void saveGraph(const std::string& s, const KeyFormatter& keyFormatter = DefaultKeyFormatter) const;

  /// @}
  /// @name Advanced Interface
  /// @{

  /** Add new factors along with their initial linearization points */
  void update(const NonlinearFactorGraph& newFactors, const Values& initialValues);

  /** Relinearization and reordering of variables */
  void reorder_relinearize();

  /** manually add a key to the end of the ordering */
  void addKey(Key key) { ordering_.push_back(key); }

  /** replace the current ordering */
  void setOrdering(const OrderingOrdered& new_ordering) { ordering_ = new_ordering; }

  /// @}

};

} // \namespace gtsam

