/*
 *  Kalman.h
 *  Embedded
 *
 *  Created by Serge on 28/4/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "LinearAlgebra.h"

namespace la {
  /**
   * A generic (read not hand-optimized) kalman filter.
   * It is parameterized by a struct with the following definitions:
   * - state_size n
   * - meas_size m
   * - value_type
   * Prior to use, the following must be set:
   * - X: initial state, n x 1
   * - A: state transition matrix, n x n
   * - H: measurement gain, m x n
   * - Q: process noise covariance, n x n
   * - R: measurement noise covariance, m x m
   * - P: state covariance, n x n
   */
  template <class Traits>
  class Kalman {
  public:
    typedef typename Traits::value_type k_real_type;
    enum {
      STATE_SIZE = Traits::state_size,
      MEAS_SIZE = Traits::meas_size,
      INPUT_SIZE = Traits::input_size
    };
    
    typedef la::Matrix<STATE_SIZE, 1, k_real_type> state_type;
    typedef la::Matrix<STATE_SIZE, STATE_SIZE, k_real_type> state_transition_type;
    typedef la::Matrix<INPUT_SIZE, 1, k_real_type> input_type;
    typedef la::Matrix<STATE_SIZE, INPUT_SIZE, k_real_type> input_to_state_type;
    typedef la::Matrix<MEAS_SIZE, 1, k_real_type> meas_type;
    typedef la::Matrix<MEAS_SIZE, STATE_SIZE, k_real_type> meas_gain_type;
    typedef la::Matrix<STATE_SIZE, STATE_SIZE, k_real_type> process_noise_covariance_type;
    typedef la::Matrix<MEAS_SIZE, MEAS_SIZE, k_real_type> meas_noise_covariance_type;
    typedef la::Matrix<STATE_SIZE, STATE_SIZE, k_real_type> prob_type;
    typedef la::Matrix<STATE_SIZE, MEAS_SIZE, k_real_type> kalman_gain_type;
    
    state_type X;
    state_transition_type A;
    input_type U;
    input_to_state_type B;
    meas_gain_type H;
    process_noise_covariance_type Q;
    meas_noise_covariance_type R;
    prob_type P;
    prob_type I;

    state_type X_predicted;
    prob_type P_predicted;
    kalman_gain_type K; 
    
    Kalman() {
      reset();
    }
  
    void reset() {
      using namespace la;
      
      // Identity
      set_identity(I);
    }
    
    void update_time() {
      using namespace la;
      
      // - Predict by linear approx X^ = AX + BU
      state_type X_from_time; mult(X_from_time, A, X);
      state_type X_from_input; mult(X_from_input, B, U);
      plus(X_predicted, X_from_time, X_from_input);
      
      // - Predict the uncertainty: P^ = A * P * A_T + Q
      state_transition_type AP; mult(AP, A, P);
      typename state_transition_type::transpose_type At; transpose(At, A);
      prob_type APAt; mult(APAt, AP, At);
      plus(P_predicted, APAt, Q);
      
      // - Compute Kalman gain
      typename meas_gain_type::transpose_type Ht; transpose(Ht, H);
      Matrix<STATE_SIZE, MEAS_SIZE, k_real_type> PHt; mult(PHt, P_predicted, Ht);
      Matrix<MEAS_SIZE, MEAS_SIZE, k_real_type>  HPHt; mult(HPHt, H, PHt);
      Matrix<MEAS_SIZE, MEAS_SIZE, k_real_type> HPHt_R; plus(HPHt_R, HPHt, R);
      Matrix<MEAS_SIZE, MEAS_SIZE, k_real_type> inv_HPHt_R; invert(inv_HPHt_R, HPHt_R);
      mult(K, PHt, inv_HPHt_R);
    }
    
    void update_measurement(const meas_type& Z) {
      using namespace la;
      // - from the measurement, calculate the innovation J = Z - H *X-
      meas_type Z_predicted; mult(Z_predicted, H, X_predicted);
      meas_type J; minus(J, Z, Z_predicted);
      
      // - Given the kalman gain, estimate the state
      state_type KInnov; mult(KInnov, K, J);
      plus(X, X_predicted, KInnov);
      
      // - And the uncertainty as well
      Matrix<STATE_SIZE, STATE_SIZE, k_real_type> KH; mult(KH, K, H);
      Matrix<STATE_SIZE, STATE_SIZE, k_real_type> I_KH; minus(I_KH, I, KH);
      mult(P, I_KH, P_predicted);
    }  
  };
}
