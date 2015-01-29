/*
 *  LinearAlgebra.h
 *  Embedded
 *
 *  Created by Serge on 24/4/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"

namespace la {
  
  template <uint32 R, uint32 C, typename T = float>
  class Matrix {
    typedef Matrix this_type;
  public:
    typedef Matrix<C, R, T> transpose_type;
    
    enum {
      ROWS = R,
      COLS = C
    };
    
    Matrix() {
    }
    
    Matrix(const T (&a)[R][C]): m(a) {
    }
    
    Matrix(const this_type& other) : m(other.m) {
    }
    
    const T& operator()(uint32 i, uint32 j) const {
      return m[i][j];
    }
    
    T& operator()(uint32 i, uint32 j) {
      return m[i][j];
    }
    
  private:
    T m[R][C];
  };
  
  /**
   * Resets a matrix to 0
   */
  template <uint32 ROWS, uint32 COLS, typename T>
  void set_zero(Matrix<COLS, ROWS, T>& to) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i)
        to(j, i) = 0;
  }
  
  /**
   * Resets a square matrix to identity
   */
  template <uint32 ROWS, typename T>
  void set_identity(Matrix<ROWS, ROWS, T>& to) {
    for (uint32 j = 0; j < ROWS; ++j)
      for (uint32 i = 0; i < ROWS; ++i)
        to(j, i) = (i == j) ? T(1) : T(0);
  }
  
  /**
   * Matrix transpose
   */
  template <uint32 ROWS, uint32 COLS, typename T>
  void transpose(Matrix<COLS, ROWS, T>& to, const Matrix<ROWS, COLS, T>& from) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i)
        to(j, i) = from(i, j);
  }
  
  /**
   * Matrix addition
   */
  template <uint32 ROWS, uint32 COLS, typename T>
  void plus(Matrix<ROWS, COLS, T>& to, const Matrix<ROWS, COLS, T>& a, const Matrix<ROWS, COLS, T>& b) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i) {
        to(i, j) = a(i,j) + b(i,j);
      }
  }
  
  /**
   * Matrix subtraction
   */
  template <uint32 ROWS, uint32 COLS, typename T>
  void minus(Matrix<ROWS, COLS, T>& to, const Matrix<ROWS, COLS, T>& a, const Matrix<ROWS, COLS, T>& b) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i) {
        to(i, j) = a(i,j) - b(i,j);
      }
  }
  
  /**
   * Matrix-scalar product
   */
  template <uint32 ROWS, uint32 COLS, typename T>
  void mult(Matrix<ROWS, COLS, T>& to, const Matrix<ROWS, COLS, T>& a, T scalar) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i) {
        to(i, j) = scalar * a(i,j);
      }
  }
  
  /**
   * Matrix multiplication
   * to = a.b
   */
  template <uint32 ROWS, uint32 COLS, uint32 K, typename T>
  void mult(Matrix<ROWS, COLS, T>& to, const Matrix<ROWS, K, T>& a, const Matrix<K, COLS, T>& b) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i) {
        to(i, j) = T(0);
        for (uint32 k = 0; k < K; ++k)
          to(i, j) += a(i, k) * b(k, j);
      }
  }
  
  /**
   * Matrix multiply by transpose
   * to = a.b^T
   */
  template <uint32 ROWS, uint32 COLS, uint32 K, typename T>
  void mult_transpose(Matrix<ROWS, COLS, T>& to, const Matrix<ROWS, K, T>& a, const Matrix<COLS, K, T>& b) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i) {
        to(i, j) = T(0);
        for (uint32 k = 0; k < K; ++k)
          to(i, j) += a(i, k) * b(j, k);
      }
  }
    
  /**
   * Matrix transpose then multiply
   * to = a^T.b
   */
  template <uint32 ROWS, uint32 COLS, uint32 K, typename T>
  void transpose_mult(Matrix<ROWS, COLS, T>& to, const Matrix<K, ROWS, T>& a, const Matrix<K, COLS, T>& b) {
    for (uint32 j = 0; j < COLS; ++j)
      for (uint32 i = 0; i < ROWS; ++i) {
        to(i, j) = T(0);
        for (uint32 k = 0; k < K; ++k)
          to(i, j) += a(k, i) * b(k, j);
      }
  }
  
  /**
   * Calculates the inverse of a matrix, if any
   * @return false if the matrix is singular, that is, if it can not be inverted
   */
#if 0
  template <uint32 DIM, typename T>
  bool invert(Matrix<DIM, DIM, T>& to, const Matrix<DIM, DIM, T>& a) {
    
    return true;
  }
#endif
  
  /**
   * Specialization for 1 by 1 matrices (scalars)
   */
  template <typename T>
  bool invert(Matrix<1, 1, T>& to, const Matrix<1, 1, T>& a, T epsilon = T(0.00001f)) {
    if (util::abs(a(0,0)) < epsilon) {
      // Matrix is singular!
      return false;
    }
    
    to(0,0) = T(1) / a(0,0);
    
    return true;
  }
  
  /**
   * Specialization for 2 by 2 matrices
   */
  template <typename T>
  bool invert(Matrix<2, 2, T>& to, const Matrix<2, 2, T>& a, T epsilon = T(0.00001f)) {
    const T determinant  = a(0,0) * a(1,1) - a(1,0) * a(0,1);
    
    if (util::abs(determinant) < epsilon) {
      // Matrix is singular!
      return false;
    }
    to(0,0) = a(1,1) / determinant;
    to(1,0) = -a(0,1) / determinant;
    to(0,1) = -a(1,0) / determinant;
    to(1,1) = a(0,0) / determinant;
    
    return true;
  }
  
}
