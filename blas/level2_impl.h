// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#include "common.h"

int EIGEN_BLAS_FUNC(gemv)(char *opa, int *m, int *n, RealScalar *palpha, RealScalar *pa, int *lda, RealScalar *pb, int *incb, RealScalar *pbeta, RealScalar *pc, int *incc)
{
  typedef void (*functype)(int, int, const Scalar *, int, const Scalar *, int , Scalar *, int, Scalar);
  static functype func[4];

  static bool init = false;
  if(!init)
  {
    for(int k=0; k<4; ++k)
      func[k] = 0;

    func[NOTR] = (internal::general_matrix_vector_product<int,Scalar,ColMajor,false,Scalar,false>::run);
    func[TR  ] = (internal::general_matrix_vector_product<int,Scalar,RowMajor,false,Scalar,false>::run);
    func[ADJ ] = (internal::general_matrix_vector_product<int,Scalar,RowMajor,Conj, Scalar,false>::run);

    init = true;
  }
  
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar* b = reinterpret_cast<Scalar*>(pb);
  Scalar* c = reinterpret_cast<Scalar*>(pc);
  Scalar alpha  = *reinterpret_cast<Scalar*>(palpha);
  Scalar beta   = *reinterpret_cast<Scalar*>(pbeta);

  // check arguments
  int info = 0;
  if(OP(*opa)==INVALID)           info = 1;
  else if(*m<0)                   info = 2;
  else if(*n<0)                   info = 3;
  else if(*lda<std::max(1,*m))    info = 6;
  else if(*incb==0)               info = 8;
  else if(*incc==0)               info = 11;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"GEMV ",&info,6);

  if(*m==0 || *n==0)
    return 0;

  int actual_m = *m;
  int actual_n = *n;
  if(OP(*opa)!=NOTR)
    std::swap(actual_m,actual_n);

  Scalar* actual_b = get_compact_vector(b,actual_n,*incb);
  Scalar* actual_c = get_compact_vector(c,actual_m,*incc);
  
  if(beta!=Scalar(1))
    vector(actual_c, actual_m) *= beta;
  
  int code = OP(*opa);
  func[code](actual_m, actual_n, a, *lda, actual_b, 1, actual_c, 1, alpha);

  if(actual_b!=b) delete[] actual_b;
  if(actual_c!=c) delete[] copy_back(actual_c,c,actual_m,*incc);

  return 1;
}

int EIGEN_BLAS_FUNC(trsv)(char *uplo, char *opa, char *diag, int *n, RealScalar *pa, int *lda, RealScalar *pb, int *incb)
{
  typedef void (*functype)(int, const Scalar *, int, Scalar *);
  static functype func[16];

  static bool init = false;
  if(!init)
  {
    for(int k=0; k<16; ++k)
      func[k] = 0;

    func[NOTR  | (UP << 2) | (NUNIT << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Upper|0,       false,ColMajor>::run);
    func[TR    | (UP << 2) | (NUNIT << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Lower|0,       false,RowMajor>::run);
    func[ADJ   | (UP << 2) | (NUNIT << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Lower|0,       Conj, RowMajor>::run);

    func[NOTR  | (LO << 2) | (NUNIT << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Lower|0,       false,ColMajor>::run);
    func[TR    | (LO << 2) | (NUNIT << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Upper|0,       false,RowMajor>::run);
    func[ADJ   | (LO << 2) | (NUNIT << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Upper|0,       Conj, RowMajor>::run);

    func[NOTR  | (UP << 2) | (UNIT  << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Upper|UnitDiag,false,ColMajor>::run);
    func[TR    | (UP << 2) | (UNIT  << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Lower|UnitDiag,false,RowMajor>::run);
    func[ADJ   | (UP << 2) | (UNIT  << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Lower|UnitDiag,Conj, RowMajor>::run);

    func[NOTR  | (LO << 2) | (UNIT  << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Lower|UnitDiag,false,ColMajor>::run);
    func[TR    | (LO << 2) | (UNIT  << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Upper|UnitDiag,false,RowMajor>::run);
    func[ADJ   | (LO << 2) | (UNIT  << 3)] = (internal::triangular_solve_vector<Scalar,Scalar,int,OnTheLeft, Upper|UnitDiag,Conj, RowMajor>::run);

    init = true;
  }

  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar* b = reinterpret_cast<Scalar*>(pb);

  int info = 0;
  if(UPLO(*uplo)==INVALID)                                            info = 1;
  else if(OP(*opa)==INVALID)                                          info = 2;
  else if(DIAG(*diag)==INVALID)                                       info = 3;
  else if(*n<0)                                                       info = 4;
  else if(*lda<std::max(1,*n))                                        info = 6;
  else if(*incb==0)                                                   info = 8;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"TRSV ",&info,6);

  Scalar* actual_b = get_compact_vector(b,*n,*incb);

  int code = OP(*opa) | (UPLO(*uplo) << 2) | (DIAG(*diag) << 3);
  func[code](*n, a, *lda, actual_b);

  if(actual_b!=b) delete[] copy_back(actual_b,b,*n,*incb);
  
  return 0;
}



int EIGEN_BLAS_FUNC(trmv)(char *uplo, char *opa, char *diag, int *n, RealScalar *pa, int *lda, RealScalar *pb, int *incb)
{
  typedef void (*functype)(int, int, const Scalar *, int, const Scalar *, int, Scalar *, int, Scalar);
  static functype func[16];

  static bool init = false;
  if(!init)
  {
    for(int k=0; k<16; ++k)
      func[k] = 0;

    func[NOTR  | (UP << 2) | (NUNIT << 3)] = (internal::product_triangular_matrix_vector<int,Upper|0,       Scalar,false,Scalar,false,ColMajor>::run);
    func[TR    | (UP << 2) | (NUNIT << 3)] = (internal::product_triangular_matrix_vector<int,Lower|0,       Scalar,false,Scalar,false,RowMajor>::run);
    func[ADJ   | (UP << 2) | (NUNIT << 3)] = (internal::product_triangular_matrix_vector<int,Lower|0,       Scalar,Conj, Scalar,false,RowMajor>::run);

    func[NOTR  | (LO << 2) | (NUNIT << 3)] = (internal::product_triangular_matrix_vector<int,Lower|0,       Scalar,false,Scalar,false,ColMajor>::run);
    func[TR    | (LO << 2) | (NUNIT << 3)] = (internal::product_triangular_matrix_vector<int,Upper|0,       Scalar,false,Scalar,false,RowMajor>::run);
    func[ADJ   | (LO << 2) | (NUNIT << 3)] = (internal::product_triangular_matrix_vector<int,Upper|0,       Scalar,Conj, Scalar,false,RowMajor>::run);

    func[NOTR  | (UP << 2) | (UNIT  << 3)] = (internal::product_triangular_matrix_vector<int,Upper|UnitDiag,Scalar,false,Scalar,false,ColMajor>::run);
    func[TR    | (UP << 2) | (UNIT  << 3)] = (internal::product_triangular_matrix_vector<int,Lower|UnitDiag,Scalar,false,Scalar,false,RowMajor>::run);
    func[ADJ   | (UP << 2) | (UNIT  << 3)] = (internal::product_triangular_matrix_vector<int,Lower|UnitDiag,Scalar,Conj, Scalar,false,RowMajor>::run);

    func[NOTR  | (LO << 2) | (UNIT  << 3)] = (internal::product_triangular_matrix_vector<int,Lower|UnitDiag,Scalar,false,Scalar,false,ColMajor>::run);
    func[TR    | (LO << 2) | (UNIT  << 3)] = (internal::product_triangular_matrix_vector<int,Upper|UnitDiag,Scalar,false,Scalar,false,RowMajor>::run);
    func[ADJ   | (LO << 2) | (UNIT  << 3)] = (internal::product_triangular_matrix_vector<int,Upper|UnitDiag,Scalar,Conj, Scalar,false,RowMajor>::run);

    init = true;
  }

  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar* b = reinterpret_cast<Scalar*>(pb);

  int info = 0;
  if(UPLO(*uplo)==INVALID)                                            info = 1;
  else if(OP(*opa)==INVALID)                                          info = 2;
  else if(DIAG(*diag)==INVALID)                                       info = 3;
  else if(*n<0)                                                       info = 4;
  else if(*lda<std::max(1,*n))                                        info = 6;
  else if(*incb==0)                                                   info = 8;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"TRMV ",&info,6);

  if(*n==0)
    return 1;

  Scalar* actual_b = get_compact_vector(b,*n,*incb);
  Matrix<Scalar,Dynamic,1> res(*n);
  res.setZero();

  int code = OP(*opa) | (UPLO(*uplo) << 2) | (DIAG(*diag) << 3);
  if(code>=16 || func[code]==0)
    return 0;

  func[code](*n, *n, a, *lda, actual_b, 1, res.data(), 1, Scalar(1));

  copy_back(res.data(),b,*n,*incb);
  if(actual_b!=b) delete[] actual_b;
  
  return 0;
}

// y = alpha*A*x + beta*y
int EIGEN_BLAS_FUNC(symv) (char *uplo, int *n, RealScalar *palpha, RealScalar *pa, int *lda, RealScalar *px, int *incx, RealScalar *pbeta, RealScalar *py, int *incy)
{
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar alpha  = *reinterpret_cast<Scalar*>(palpha);
  Scalar beta   = *reinterpret_cast<Scalar*>(pbeta);
  
  // check arguments
  int info = 0;
  if(UPLO(*uplo)==INVALID)        info = 1;
  else if(*n<0)                   info = 2;
  else if(*lda<std::max(1,*n))    info = 5;
  else if(*incx==0)               info = 7;
  else if(*incy==0)               info = 10;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"SYMV ",&info,6);

  if(*n==0)
    return 0;

  Scalar* actual_x = get_compact_vector(x,*n,*incx);
  Scalar* actual_y = get_compact_vector(y,*n,*incy);

  if(beta!=Scalar(1))
    vector(actual_y, *n) *= beta;

  // TODO performs a direct call to the underlying implementation function
       if(UPLO(*uplo)==UP) vector(actual_y,*n).noalias() += matrix(a,*n,*n,*lda).selfadjointView<Upper>() * (alpha * vector(actual_x,*n));
  else if(UPLO(*uplo)==LO) vector(actual_y,*n).noalias() += matrix(a,*n,*n,*lda).selfadjointView<Lower>() * (alpha * vector(actual_x,*n));

  if(actual_x!=x) delete[] actual_x;
  if(actual_y!=y) delete[] copy_back(actual_y,y,*n,*incy);

  return 1;
}

// C := alpha*x*x' + C
int EIGEN_BLAS_FUNC(syr)(char *uplo, int *n, RealScalar *palpha, RealScalar *px, int *incx, RealScalar *pc, int *ldc)
{
  
//   typedef void (*functype)(int, const Scalar *, int, Scalar *, int, Scalar);
//   static functype func[2];

//   static bool init = false;
//   if(!init)
//   {
//     for(int k=0; k<2; ++k)
//       func[k] = 0;
// 
//     func[UP] = (internal::selfadjoint_product<Scalar,ColMajor,ColMajor,false,UpperTriangular>::run);
//     func[LO] = (internal::selfadjoint_product<Scalar,ColMajor,ColMajor,false,LowerTriangular>::run);

//     init = true;
//   }

  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* c = reinterpret_cast<Scalar*>(pc);
  Scalar alpha = *reinterpret_cast<Scalar*>(palpha);
  
  int info = 0;
  if(UPLO(*uplo)==INVALID)                                            info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*ldc<std::max(1,*n))                                        info = 7;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"SYR  ",&info,6);
  
  if(alpha==Scalar(0))
    return 1;
  
  // if the increment is not 1, let's copy it to a temporary vector to enable vectorization
  Scalar* x_cpy = get_compact_vector(x,*n,*incx);
    
  // TODO perform direct calls to underlying implementation
  if(UPLO(*uplo)==LO)       matrix(c,*n,*n,*ldc).selfadjointView<Lower>().rankUpdate(vector(x_cpy,*n), alpha);
  else if(UPLO(*uplo)==UP)  matrix(c,*n,*n,*ldc).selfadjointView<Upper>().rankUpdate(vector(x_cpy,*n), alpha);
  
  if(x_cpy!=x)  delete[] x_cpy;

//   func[code](*n, a, *inca, c, *ldc, alpha);
  return 1;
}


// C := alpha*x*y' + alpha*y*x' + C
int EIGEN_BLAS_FUNC(syr2)(char *uplo, int *n, RealScalar *palpha, RealScalar *px, int *incx, RealScalar *py, int *incy, RealScalar *pc, int *ldc)
{
//   typedef void (*functype)(int, const Scalar *, int, const Scalar *, int, Scalar *, int, Scalar);
//   static functype func[2];
// 
//   static bool init = false;
//   if(!init)
//   {
//     for(int k=0; k<2; ++k)
//       func[k] = 0;
// 
//     func[UP] = (internal::selfadjoint_product<Scalar,ColMajor,ColMajor,false,UpperTriangular>::run);
//     func[LO] = (internal::selfadjoint_product<Scalar,ColMajor,ColMajor,false,LowerTriangular>::run);
// 
//     init = true;
//   }

  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar* c = reinterpret_cast<Scalar*>(pc);
  Scalar alpha = *reinterpret_cast<Scalar*>(palpha);
  
  int info = 0;
  if(UPLO(*uplo)==INVALID)                                            info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*incy==0)                                                   info = 7;
  else if(*ldc<std::max(1,*n))                                        info = 9;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"SYR2 ",&info,6);
  
  if(alpha==Scalar(0))
    return 1;
  
  Scalar* x_cpy = get_compact_vector(x,*n,*incx);
  Scalar* y_cpy = get_compact_vector(y,*n,*incy);
    
  // TODO perform direct calls to underlying implementation
  if(UPLO(*uplo)==LO)       matrix(c,*n,*n,*ldc).selfadjointView<Lower>().rankUpdate(vector(x_cpy,*n), vector(y_cpy,*n), alpha);
  else if(UPLO(*uplo)==UP)  matrix(c,*n,*n,*ldc).selfadjointView<Upper>().rankUpdate(vector(x_cpy,*n), vector(y_cpy,*n), alpha);
  
  if(x_cpy!=x)  delete[] x_cpy;
  if(y_cpy!=y)  delete[] y_cpy;

//   int code = UPLO(*uplo);
//   if(code>=2 || func[code]==0)
//     return 0;

//   func[code](*n, a, *inca, b, *incb, c, *ldc, alpha);
  return 1;
}

/**  DGBMV  performs one of the matrix-vector operations
  *
  *     y := alpha*A*x + beta*y,   or   y := alpha*A'*x + beta*y,
  *
  *  where alpha and beta are scalars, x and y are vectors and A is an
  *  m by n band matrix, with kl sub-diagonals and ku super-diagonals.
  */
int EIGEN_BLAS_FUNC(gbmv)(char *trans, int *m, int *n, int *kl, int *ku, RealScalar *alpha, RealScalar *a, int *lda,
                          RealScalar *x, int *incx, RealScalar *beta, RealScalar *y, int *incy)
{
  return 1;
}
/**  DSBMV  performs the matrix-vector  operation
  *
  *     y := alpha*A*x + beta*y,
  *
  *  where alpha and beta are scalars, x and y are n element vectors and
  *  A is an n by n symmetric band matrix, with k super-diagonals.
  */
int EIGEN_BLAS_FUNC(sbmv)( char *uplo, int *n, int *k, RealScalar *alpha, RealScalar *a, int *lda,
                           RealScalar *x, int *incx, RealScalar *beta, RealScalar *y, int *incy)
{
  return 1;
}

/**  DTBMV  performs one of the matrix-vector operations
  *
  *     x := A*x,   or   x := A'*x,
  *
  *  where x is an n element vector and  A is an n by n unit, or non-unit,
  *  upper or lower triangular band matrix, with ( k + 1 ) diagonals.
  */
int EIGEN_BLAS_FUNC(tbmv)(char *uplo, char *trans, char *diag, int *n, int *k, RealScalar *a, int *lda, RealScalar *x, int *incx)
{
  return 1;
}

/**  DTBSV  solves one of the systems of equations
  *
  *     A*x = b,   or   A'*x = b,
  *
  *  where b and x are n element vectors and A is an n by n unit, or
  *  non-unit, upper or lower triangular band matrix, with ( k + 1 )
  *  diagonals.
  *
  *  No test for singularity or near-singularity is included in this
  *  routine. Such tests must be performed before calling this routine.
  */
int EIGEN_BLAS_FUNC(tbsv)(char *uplo, char *trans, char *diag, int *n, int *k, RealScalar *a, int *lda, RealScalar *x, int *incx)
{
  return 1;
}

/**  DSPMV  performs the matrix-vector operation
  *
  *     y := alpha*A*x + beta*y,
  *
  *  where alpha and beta are scalars, x and y are n element vectors and
  *  A is an n by n symmetric matrix, supplied in packed form.
  *
  */
int EIGEN_BLAS_FUNC(spmv)(char *uplo, int *n, RealScalar *alpha, RealScalar *ap, RealScalar *x, int *incx, RealScalar *beta, RealScalar *y, int *incy)
{
  return 1;
}

/**  DTPMV  performs one of the matrix-vector operations
  *
  *     x := A*x,   or   x := A'*x,
  *
  *  where x is an n element vector and  A is an n by n unit, or non-unit,
  *  upper or lower triangular matrix, supplied in packed form.
  */
int EIGEN_BLAS_FUNC(tpmv)(char *uplo, char *trans, char *diag, int *n, RealScalar *ap, RealScalar *x, int *incx)
{
  return 1;
}

/**  DTPSV  solves one of the systems of equations
  *
  *     A*x = b,   or   A'*x = b,
  *
  *  where b and x are n element vectors and A is an n by n unit, or
  *  non-unit, upper or lower triangular matrix, supplied in packed form.
  *
  *  No test for singularity or near-singularity is included in this
  *  routine. Such tests must be performed before calling this routine.
  */
int EIGEN_BLAS_FUNC(tpsv)(char *uplo, char *trans, char *diag, int *n, RealScalar *ap, RealScalar *x, int *incx)
{
  return 1;
}

/**  DGER   performs the rank 1 operation
  *
  *     A := alpha*x*y' + A,
  *
  *  where alpha is a scalar, x is an m element vector, y is an n element
  *  vector and A is an m by n matrix.
  */
int EIGEN_BLAS_FUNC(ger)(int *m, int *n, Scalar *palpha, Scalar *px, int *incx, Scalar *py, int *incy, Scalar *pa, int *lda)
{
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar alpha = *reinterpret_cast<Scalar*>(palpha);
  
  int info = 0;
       if(*m<0)                                                       info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*incy==0)                                                   info = 7;
  else if(*lda<std::max(1,*m))                                        info = 9;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"GER  ",&info,6);
  
  if(alpha==Scalar(0))
    return 1;
  
  Scalar* x_cpy = get_compact_vector(x,*m,*incx);
  Scalar* y_cpy = get_compact_vector(y,*n,*incy);
    
  // TODO perform direct calls to underlying implementation
  matrix(a,*m,*n,*lda) += alpha * vector(x_cpy,*m) * vector(y_cpy,*n).adjoint();
  
  if(x_cpy!=x)  delete[] x_cpy;
  if(y_cpy!=y)  delete[] y_cpy;
  
  return 1;
}

/**  DSPR    performs the symmetric rank 1 operation
  *
  *     A := alpha*x*x' + A,
  *
  *  where alpha is a real scalar, x is an n element vector and A is an
  *  n by n symmetric matrix, supplied in packed form.
  */
int EIGEN_BLAS_FUNC(spr)(char *uplo, int *n, Scalar *alpha, Scalar *x, int *incx, Scalar *ap)
{
  return 1;
}
/**  DSPR2  performs the symmetric rank 2 operation
  *
  *     A := alpha*x*y' + alpha*y*x' + A,
  *
  *  where alpha is a scalar, x and y are n element vectors and A is an
  *  n by n symmetric matrix, supplied in packed form.
  */
int EIGEN_BLAS_FUNC(spr2)(char *uplo, int *n, RealScalar *alpha, RealScalar *x, int *incx, RealScalar *y, int *incy, RealScalar *ap)
{
  return 1;
}

#if ISCOMPLEX
/**  ZHEMV  performs the matrix-vector  operation
  *
  *     y := alpha*A*x + beta*y,
  *
  *  where alpha and beta are scalars, x and y are n element vectors and
  *  A is an n by n hermitian matrix.
  */
int EIGEN_BLAS_FUNC(hemv)(char *uplo, int *n, RealScalar *palpha, RealScalar *pa, int *lda, RealScalar *px, int *incx, RealScalar *pbeta, RealScalar *py, int *incy)
{
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar alpha  = *reinterpret_cast<Scalar*>(palpha);
  Scalar beta   = *reinterpret_cast<Scalar*>(pbeta);

  // check arguments
  int info = 0;
  if(UPLO(*uplo)==INVALID)        info = 1;
  else if(*n<0)                   info = 2;
  else if(*lda<std::max(1,*n))    info = 5;
  else if(*incx==0)               info = 7;
  else if(*incy==0)               info = 10;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"HEMV ",&info,6);

  if(*n==0)
    return 1;

  Scalar* actual_x = get_compact_vector(x,*n,*incx);
  Scalar* actual_y = get_compact_vector(y,*n,*incy);

  if(beta!=Scalar(1))
    vector(actual_y, *n) *= beta;

  if(alpha!=Scalar(0))
  {
    // TODO performs a direct call to the underlying implementation function
         if(UPLO(*uplo)==UP) vector(actual_y,*n).noalias() += matrix(a,*n,*n,*lda).selfadjointView<Upper>() * (alpha * vector(actual_x,*n));
    else if(UPLO(*uplo)==LO) vector(actual_y,*n).noalias() += matrix(a,*n,*n,*lda).selfadjointView<Lower>() * (alpha * vector(actual_x,*n));
  }
  
  if(actual_x!=x) delete[] actual_x;
  if(actual_y!=y) delete[] copy_back(actual_y,y,*n,*incy);

  return 1;
}

/**  ZHBMV  performs the matrix-vector  operation
  *
  *     y := alpha*A*x + beta*y,
  *
  *  where alpha and beta are scalars, x and y are n element vectors and
  *  A is an n by n hermitian band matrix, with k super-diagonals.
  */
int EIGEN_BLAS_FUNC(hbmv)(char *uplo, int *n, int *k, RealScalar *alpha, RealScalar *a, int *lda,
                          RealScalar *x, int *incx, RealScalar *beta, RealScalar *y, int *incy)
{
  return 1;
}

/**  ZHPMV  performs the matrix-vector operation
  *
  *     y := alpha*A*x + beta*y,
  *
  *  where alpha and beta are scalars, x and y are n element vectors and
  *  A is an n by n hermitian matrix, supplied in packed form.
  */
int EIGEN_BLAS_FUNC(hpmv)(char *uplo, int *n, RealScalar *alpha, RealScalar *ap, RealScalar *x, int *incx, RealScalar *beta, RealScalar *y, int *incy)
{
  return 1;
}

/**  ZHPR    performs the hermitian rank 1 operation
  *
  *     A := alpha*x*conjg( x' ) + A,
  *
  *  where alpha is a real scalar, x is an n element vector and A is an
  *  n by n hermitian matrix, supplied in packed form.
  */
int EIGEN_BLAS_FUNC(hpr)(char *uplo, int *n, RealScalar *alpha, RealScalar *x, int *incx, RealScalar *ap)
{
  return 1;
}

/**  ZHPR2  performs the hermitian rank 2 operation
  *
  *     A := alpha*x*conjg( y' ) + conjg( alpha )*y*conjg( x' ) + A,
  *
  *  where alpha is a scalar, x and y are n element vectors and A is an
  *  n by n hermitian matrix, supplied in packed form.
  */
int EIGEN_BLAS_FUNC(hpr2)(char *uplo, int *n, RealScalar *palpha, RealScalar *x, int *incx, RealScalar *y, int *incy, RealScalar *ap)
{
  return 1;
}

/**  ZHER   performs the hermitian rank 1 operation
  *
  *     A := alpha*x*conjg( x' ) + A,
  *
  *  where alpha is a real scalar, x is an n element vector and A is an
  *  n by n hermitian matrix.
  */
int EIGEN_BLAS_FUNC(her)(char *uplo, int *n, RealScalar *palpha, RealScalar *px, int *incx, RealScalar *pa, int *lda)
{
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  RealScalar alpha = *reinterpret_cast<RealScalar*>(palpha);
  
  int info = 0;
  if(UPLO(*uplo)==INVALID)                                            info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*lda<std::max(1,*n))                                        info = 7;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"HER  ",&info,6);
  
  if(alpha==RealScalar(0))
    return 1;
  
  Scalar* x_cpy = get_compact_vector(x, *n, *incx);
  
  // TODO perform direct calls to underlying implementation
  if(UPLO(*uplo)==LO)       matrix(a,*n,*n,*lda).selfadjointView<Lower>().rankUpdate(vector(x_cpy,*n), alpha);
  else if(UPLO(*uplo)==UP)  matrix(a,*n,*n,*lda).selfadjointView<Upper>().rankUpdate(vector(x_cpy,*n), alpha);
  
  matrix(a,*n,*n,*lda).diagonal().imag().setZero();
  
  if(x_cpy!=x)  delete[] x_cpy;

  return 1;
}

/**  ZHER2  performs the hermitian rank 2 operation
  *
  *     A := alpha*x*conjg( y' ) + conjg( alpha )*y*conjg( x' ) + A,
  *
  *  where alpha is a scalar, x and y are n element vectors and A is an n
  *  by n hermitian matrix.
  */
int EIGEN_BLAS_FUNC(her2)(char *uplo, int *n, RealScalar *palpha, RealScalar *px, int *incx, RealScalar *py, int *incy, RealScalar *pa, int *lda)
{
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar alpha = *reinterpret_cast<Scalar*>(palpha);
  
  int info = 0;
  if(UPLO(*uplo)==INVALID)                                            info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*incy==0)                                                   info = 7;
  else if(*lda<std::max(1,*n))                                        info = 9;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"HER2 ",&info,6);
  
  if(alpha==Scalar(0))
    return 1;
  
  Scalar* x_cpy = get_compact_vector(x, *n, *incx);
  Scalar* y_cpy = get_compact_vector(y, *n, *incy);
  
  // TODO perform direct calls to underlying implementation
  if(UPLO(*uplo)==LO)       matrix(a,*n,*n,*lda).selfadjointView<Lower>().rankUpdate(vector(x_cpy,*n),vector(y_cpy,*n),alpha);
  else if(UPLO(*uplo)==UP)  matrix(a,*n,*n,*lda).selfadjointView<Upper>().rankUpdate(vector(x_cpy,*n),vector(y_cpy,*n),alpha);
  
  matrix(a,*n,*n,*lda).diagonal().imag().setZero();
  
  if(x_cpy!=x)  delete[] x_cpy;
  if(y_cpy!=y)  delete[] y_cpy;

  return 1;
}

/**  ZGERU  performs the rank 1 operation
  *
  *     A := alpha*x*y' + A,
  *
  *  where alpha is a scalar, x is an m element vector, y is an n element
  *  vector and A is an m by n matrix.
  */
int EIGEN_BLAS_FUNC(geru)(int *m, int *n, RealScalar *palpha, RealScalar *px, int *incx, RealScalar *py, int *incy, RealScalar *pa, int *lda)
{
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar alpha = *reinterpret_cast<Scalar*>(palpha);
  
  int info = 0;
       if(*m<0)                                                       info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*incy==0)                                                   info = 7;
  else if(*lda<std::max(1,*m))                                        info = 9;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"GERU ",&info,6);
  
  if(alpha==Scalar(0))
    return 1;
  
  Scalar* x_cpy = get_compact_vector(x,*m,*incx);
  Scalar* y_cpy = get_compact_vector(y,*n,*incy);
    
  // TODO perform direct calls to underlying implementation
  matrix(a,*m,*n,*lda) += alpha * vector(x_cpy,*m) * vector(y_cpy,*n).transpose();
  
  if(x_cpy!=x)  delete[] x_cpy;
  if(y_cpy!=y)  delete[] y_cpy;
  
  return 1;
}

/**  ZGERC  performs the rank 1 operation
  *
  *     A := alpha*x*conjg( y' ) + A,
  *
  *  where alpha is a scalar, x is an m element vector, y is an n element
  *  vector and A is an m by n matrix.
  */
int EIGEN_BLAS_FUNC(gerc)(int *m, int *n, RealScalar *palpha, RealScalar *px, int *incx, RealScalar *py, int *incy, RealScalar *pa, int *lda)
{
  Scalar* x = reinterpret_cast<Scalar*>(px);
  Scalar* y = reinterpret_cast<Scalar*>(py);
  Scalar* a = reinterpret_cast<Scalar*>(pa);
  Scalar alpha = *reinterpret_cast<Scalar*>(palpha);
  
  int info = 0;
       if(*m<0)                                                       info = 1;
  else if(*n<0)                                                       info = 2;
  else if(*incx==0)                                                   info = 5;
  else if(*incy==0)                                                   info = 7;
  else if(*lda<std::max(1,*m))                                        info = 9;
  if(info)
    return xerbla_(SCALAR_SUFFIX_UP"GERC ",&info,6);
  
  if(alpha==Scalar(0))
    return 1;
  
  Scalar* x_cpy = get_compact_vector(x,*m,*incx);
  Scalar* y_cpy = get_compact_vector(y,*n,*incy);
    
  // TODO perform direct calls to underlying implementation
  matrix(a,*m,*n,*lda) += alpha * vector(x_cpy,*m) * vector(y_cpy,*n).adjoint();
  
  if(x_cpy!=x)  delete[] x_cpy;
  if(y_cpy!=y)  delete[] y_cpy;
  
  return 1;
}
#endif // ISCOMPLEX
