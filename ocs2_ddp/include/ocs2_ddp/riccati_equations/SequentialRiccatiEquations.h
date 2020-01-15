/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

 * Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#pragma once

#include <Eigen/Dense>
#include <Eigen/StdVector>
#include <vector>

#include <ocs2_core/Dimensions.h>
#include <ocs2_core/integration/OdeBase.h>
#include <ocs2_core/misc/LinearInterpolation.h>
#include <ocs2_core/model_data/ModelDataBase.h>

#include "ocs2_ddp/riccati_equations/RiccatiModification.h"

namespace ocs2 {

/**
 * Helper function to define the s vector dimension, also supports dynamic size -1;
 *
 * @param [in] state_dim: Dimension of the state space.
 * @return Dimension of the flattened and concatenated vector from Sm, Sv and s.
 */
static constexpr int s_vector_dim(int state_dim) {
  /** If STATE_DIM=n, Then: n(n+1)/2 entries from triangular matrix Sm, n entries from vector Sv and +1 one from a scalar */
  return state_dim == Eigen::Dynamic ? Eigen::Dynamic : (state_dim * (state_dim + 1) / 2 + state_dim + 1);
}

/**
 * This class implements the time-normalized Riccati equations for SLQ problem.
 *
 * @tparam STATE_DIM: Dimension of the state space.
 * @tparam INPUT_DIM: Dimension of the control input space.
 */
template <int STATE_DIM, int INPUT_DIM>
class SequentialRiccatiEquations final : public OdeBase<s_vector_dim(STATE_DIM)> {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  static constexpr int S_DIM_ = s_vector_dim(STATE_DIM);
  using BASE = OdeBase<S_DIM_>;

  using DIMENSIONS = Dimensions<STATE_DIM, INPUT_DIM>;
  using scalar_t = typename DIMENSIONS::scalar_t;
  using scalar_array_t = typename DIMENSIONS::scalar_array_t;
  using size_array_t = typename DIMENSIONS::size_array_t;
  using state_vector_t = typename DIMENSIONS::state_vector_t;
  using state_vector_array_t = typename DIMENSIONS::state_vector_array_t;
  using input_vector_t = typename DIMENSIONS::input_vector_t;
  using input_vector_array_t = typename DIMENSIONS::input_vector_array_t;
  using input_state_matrix_t = typename DIMENSIONS::input_state_matrix_t;
  using input_state_matrix_array_t = typename DIMENSIONS::input_state_matrix_array_t;
  using state_matrix_t = typename DIMENSIONS::state_matrix_t;
  using state_matrix_array_t = typename DIMENSIONS::state_matrix_array_t;
  using input_matrix_t = typename DIMENSIONS::input_matrix_t;
  using input_matrix_array_t = typename DIMENSIONS::input_matrix_array_t;
  using state_input_matrix_t = typename DIMENSIONS::state_input_matrix_t;
  using state_input_matrix_array_t = typename DIMENSIONS::state_input_matrix_array_t;
  using dynamic_vector_t = typename DIMENSIONS::dynamic_vector_t;
  using dynamic_matrix_t = typename DIMENSIONS::dynamic_matrix_t;
  using dynamic_matrix_array_t = typename DIMENSIONS::dynamic_matrix_array_t;
  using dynamic_vector_array_t = typename DIMENSIONS::dynamic_vector_array_t;

  using riccati_modification_t = RiccatiModification<STATE_DIM, INPUT_DIM>;

  using s_vector_t = Eigen::Matrix<scalar_t, S_DIM_, 1>;
  using s_vector_array_t = std::vector<s_vector_t, Eigen::aligned_allocator<s_vector_t> >;

  /**
   * Constructor.
   */
  explicit SequentialRiccatiEquations(bool preComputeRiccatiTerms = true);

  /**
   * Default destructor.
   */
  ~SequentialRiccatiEquations() = default;

  /**
   * Transcribe symmetric matrix Sm, vector Sv and scalar s into a single vector.
   *
   * @param [in] Sm: \f$ S_m \f$
   * @param [in] Sv: \f$ S_v \f$
   * @param [in] s: \f$ s \f$
   * @param [out] allSs: Single vector constructed by concatenating Sm, Sv and s.
   */
  static void convert2Vector(const state_matrix_t& Sm, const state_vector_t& Sv, const scalar_t& s, s_vector_t& allSs);

  /**
   * Transcribes the stacked vector allSs into a symmetric matrix, Sm, a vector, Sv and a single scalar, s.
   *
   * @param [in] allSs: Single vector constructed by concatenating Sm, Sv and s.
   * @param [out] Sm: \f$ S_m \f$
   * @param [out] Sv: \f$ S_v \f$
   * @param [out] s: \f$ s \f$
   */
  static void convert2Matrix(const s_vector_t& allSs, state_matrix_t& Sm, state_vector_t& Sv, scalar_t& s);

  /**
   * Sets coefficients of the model.
   *
   * @param [in] timeStampPtr: A pointer to the time stamp trajectory.
   * @param [in] ModelDataBase: A pointer to the model data trajectory.
   * @param [in] AmPtr: A pointer to the trajectory of \f$ A_m(t) \f$ .
   * @param [in] QvPtr: A pointer to the trajectory of \f$ Q_v(t) \f$ .
   * @param [in] QmPtr: A pointer to the trajectory of \f$ Q_m(t) \f$ .
   * @param [in] RmInversePtr: A pointer to the trajectory of \f$ R_m^{-1}(t) \f$ .
   * @param [in] RmPtr: A pointer to the trajectory of \f$ R_m(t) \f$ .
   */
  void setData(const scalar_array_t* timeStampPtr, const ModelDataBase::array_t* modelDataPtr,
               const ModelDataBase::array_t* projectedModelDataPtr, const dynamic_matrix_array_t* RinvCholPtr,
               const size_array_t* eventsPastTheEndIndecesPtr, const ModelDataBase::array_t* modelDataEventTimesPtr,
               const riccati_modification_t* riccatiModificationPtr);

  /**
   * Riccati jump map at switching moments
   *
   * @param [in] time: Normalized transition time
   * @param [in] state: transition state
   * @param [out] mappedState: mapped state after transition
   */
  void computeJumpMap(const scalar_t& z, const s_vector_t& state, s_vector_t& mappedState) override;

  /**
   * Computes derivatives.
   *
   * @param [in] z: Normalized time.
   * @param [in] allSs: Single vector constructed by concatenating Sm, Sv and s.
   * @param [out] derivatives: d(allSs)/dz.
   */
  void computeFlowMap(const scalar_t& z, const s_vector_t& allSs, s_vector_t& derivatives) override;

 private:
  bool preComputeRiccatiTerms_;

  // array pointers
  const scalar_array_t* timeStampPtr_;
  const ModelDataBase::array_t* modelDataPtr_;
  const ModelDataBase::array_t* projectedModelDataPtr_;
  const dynamic_matrix_array_t* RinvCholPtr_;

  scalar_array_t eventTimes_;
  const ModelDataBase::array_t* modelDataEventTimesPtr_;

  const riccati_modification_t* riccatiModificationPtr_;

  // Arrays to store precomputation
  dynamic_matrix_array_t Qm_minus_P_Rinv_P_array_;
  dynamic_vector_array_t Qv_minus_P_Rinv_Rv_array_;
  dynamic_matrix_array_t AmT_minus_P_Rinv_B_array_;
  dynamic_matrix_array_t B_RinvChol_array_;
  dynamic_vector_array_t RinvCholT_Rv_array_;

  // members required only in computeFlowMap()
  state_matrix_t Sm_;
  state_vector_t Sv_;
  scalar_t s_;

  dynamic_matrix_t Qm_;
  dynamic_vector_t Qv_;
  scalar_t q_;
  dynamic_matrix_t AmT_minus_P_Rinv_Bm_;
  dynamic_matrix_t B_RinvChol_;
  dynamic_vector_t RinvCholT_Rv_;
  dynamic_matrix_t SmT_B_RinvChol_;
  state_matrix_t AmT_Sm_;
  dynamic_matrix_t Am_;
  dynamic_matrix_t Bm_;
  dynamic_vector_t Rv_;
  dynamic_matrix_t RinvChol_;
  dynamic_matrix_t Pm_;

  state_matrix_t deltaQm_;
  input_matrix_t deltaRm_;
  input_state_matrix_t deltaPm_;
};

extern template class SequentialRiccatiEquations<Eigen::Dynamic, Eigen::Dynamic>;

}  // namespace ocs2

#include <ocs2_ddp/riccati_equations/implementation/SequentialRiccatiEquations.h>
