/*
 *  LeastSquaresEstimator.h
 *  Embedded
 *
 *  Created by Serge on 5/23/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */

/*
 function [xn, Pn] = rec_least_squares(x, P, at, y, l)
 pp = at * P;
 yy = l + pp * transpose(at);
 k = transpose(pp) / yy;
 innov = y - at * x;  %# transpose(x) * transpose(at);  % Innovation
 xn = x + k * innov;
 PP = k * pp;
 Pn = inv(l) * (P - PP);
 end 
*/

#pragma once
#include "LinearAlgebra.h"

namespace la {
  struct LeastSquaresEstimator_Scalar_Traits {
    enum {
      NB_STATES = 1,
      NB_MEAS = 1,
      NB_OUTPUTS = 1
    };
    typedef float real_type;
  };
  
  struct LeastSquaresEstimator_Affine_Traits {
    enum {
      NB_STATES = 2,  // (origin, slope)
      NB_MEAS = 2,  // (1, x)
      NB_OUTPUTS = 1  // y value
    };
    typedef float real_type;
  };
  
  /**
   * Performs recursive least squares with forgetting factor
   * See http://www.cs.tut.fi/~tabus/course/ASP/LectureNew10.pdf
   */
  template <typename T>
  class LeastSquaresEstimator {
  public:
    typedef LeastSquaresEstimator this_type;
    typedef typename T::real_type real_type;
    typedef Matrix<T::NB_STATES, 1, real_type> x_type;
    typedef Matrix<T::NB_OUTPUTS, 1, real_type> y_type;
    typedef Matrix<T::NB_MEAS, 1, real_type> a_type;
    typedef Matrix<T::NB_STATES, T::NB_STATES, real_type> P_type;

    LeastSquaresEstimator(real_type x_variance, real_type lambda = 1)
    : m_lambda(lambda) {
      set_identity(m_P);
      mult(m_P, m_P, x_variance * 100);
    }
    
    this_type& set_state(const x_type& x) {
      m_x = x;
      return *this;
    }
    
    const x_type& get_state() const {
      return m_x;
    }
    
    x_type& get_state() {
      return m_x;
    }
    
    real_type get_state(uint32 i) const {
      return m_x(i, 0);
    }
    
    this_type& set_state(uint32 i, real_type c) {
      m_x(i, 0) = c;
      return *this;
    }
    
    P_type& get_cov() {
      return m_P;
    }
    
    real_type get_cov(uint32 row, uint32 col) const {
      return m_P(row, col);
    }
    
    this_type& set_cov(uint32 row, uint32 col, real_type c) {
      m_P(row, col) = c;
      return *this;
    }
    
    this_type& set_lambda(real_type lambda) {
      m_lambda = lambda;
      return *this;
    }
    
    real_type get_lambda() const {
      return m_lambda;
    }
    
    this_type& update(const y_type& y, const a_type& a) {
      const real_type inv_lambda = 1 / m_lambda;
      
      // pp = aT*P
      typename a_type::transpose_type pp;
      transpose_mult(pp, a, m_P);
      
      // inv_yy = 1 / (l + (aT*P*a))
      Matrix<1, 1, real_type> yy;
      mult(yy, pp, a);
      const real_type inv_yy = 1 / (m_lambda + yy(0, 0));
      
      // ppt = (aT*P)T
      a_type ppt;
      transpose(ppt, pp);
      
      // k = (aT*P)T / (l + (aT*P*a))
//#error  ??????????? should be P*a / (l + (aT*P*a))  OR (aT * PT)T / () since MV = (VT MT)T ????
      a_type k;
      mult(k, ppt, inv_yy);
      
      // innovation = y - aT * x
      y_type innovation;
      transpose_mult(innovation, a, m_x);
      minus(innovation, y, innovation);
      
      // Scale innovation back to x
      x_type x_innovation;
      mult(x_innovation, k, innovation);
      
      // Update the state x
      plus(m_x, m_x, x_innovation);
      
      // Change in uncertainty
      P_type p_change;
      mult(p_change, k, pp);
      
      // New P
      P_type new_P;
      minus(new_P, m_P, p_change);
      
      // Update P; raise uncertainty w.r.t the forgetting factor
      mult(m_P, new_P, inv_lambda);
      
      return *this;
    }
    
  private:
    real_type m_lambda; // Forgetting factor
    x_type m_x;
    P_type m_P;
  };
  
  /**
   * Version of the least squares optimized for estimation of a scalar
   */
  template <typename T = float>
  class LeastSquaresScalarEstimator {
    typedef T real_type;
    typedef real_type x_type;
    typedef real_type y_type;
    
  public:
    LeastSquaresScalarEstimator(real_type x_variance, real_type lambda = 1)
    : m_lambda(lambda) {
      m_variance = x_variance * 100;
    }
    
    void set_state(const x_type& x) {
      m_x = x;
    }
    
    const x_type& get_state() const {
      return m_x;
    }
    
    x_type& get_state() {
      return m_x;
    }
    
    void set_lambda(real_type lambda) {
      m_lambda = lambda;
    }
    
    real_type get_lambda() const {
      return m_lambda;
    }
    
    /**
     * No measurement. For a scalar constant, the factor is set to 1
     */
    void update(const y_type& y) {
      const real_type inv_lambda = 1 / m_lambda;

      const real_type innovation = y - m_x;
      
      const real_type k = m_lambda * m_variance / (m_lambda + m_variance);
      
      // Update the estimation
      m_x += k * innovation;
      // Update the variance
      m_variance *= inv_lambda * (1 - k);
    }
    
  private:
    real_type m_lambda; // Forgetting factor
    x_type m_x;
    T m_variance;
  };
}
