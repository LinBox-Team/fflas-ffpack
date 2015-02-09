/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 the FFLAS-FFPACK group
 *
 * Written by   BB <bbboyer@ncsu.edu>
 *              Bastien Vialla <bastien.vialla@lirmm.fr>
 *
 *
 * ========LICENCE========
 * This file is part of the library FFLAS-FFPACK.
 *
 * FFLAS-FFPACK is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 * ========LICENCE========
 *.
 */

/** @file fflas/fflas_sparse.h
*/

#ifndef __FFLASFFPACK_fflas_fflas_sparse_H
#define __FFLASFFPACK_fflas_fflas_sparse_H

#ifndef index_t
#define index_t uint32_t
#endif

// Bigger multiple of s lesser or equal than x, s must be a power of two
#ifndef ROUND_DOWN
#define ROUND_DOWN(x, s) ((x) & ~((s) - 1))
#endif

#ifndef __FFLASFFPACK_CACHE_LINE_SIZE
#define __FFLASFFPACK_CACHE_LINE_SIZE 64
#endif

#if __GNUC_MINOR__ >= 7
#define assume_aligned(pout, pin, v)                                                                                   \
    decltype(pin) pout = static_cast<decltype(pin)>(__builtin_assume_aligned(pin, v));
#else
#define assume_aligned(pout, pin, v) decltype(pin) pout = pin;
#endif

#define DENSE_THRESHOLD 0.5

#include "fflas-ffpack/config.h"
#include "fflas-ffpack/config-blas.h"
#include "fflas-ffpack/field/field-traits.h"
#include <type_traits>
#include <vector>

#ifdef __FFLASFFPACK_HAVE_MKL
#ifndef _MKL_H_ // temporary
#error "MKL (mkl.h) not present, while you have MKL enabled"
#endif
#undef index_t
#define index_t MKL_INT
#endif

namespace FFLAS {

enum class SparseMatrix_t {
    CSR,
    CSR_ZO,
    COO,
    COO_ZO,
    ELL,
    ELL_ZO,
    SELL,
    SELL_ZO,
    ELL_simd,
    ELL_simd_ZO,
    CSR_HYB,
    HYB_ZO
};

template <class Field, SparseMatrix_t> struct Sparse;

} // FFLAS

#include "fflas-ffpack/fflas/fflas_sparse/csr.h"
#include "fflas-ffpack/fflas/fflas_sparse/coo.h"
#include "fflas-ffpack/fflas/fflas_sparse/ell.h"
#include "fflas-ffpack/fflas/fflas_sparse/sell.h"
#include "fflas-ffpack/fflas/fflas_sparse/csr_hyb.h"
#include "fflas-ffpack/fflas/fflas_sparse/ell_simd.h"
#include "fflas-ffpack/fflas/fflas_sparse/hyb_zo.h"

namespace FFLAS {
/****************************
 *
 *  SparseMatrix Traits
 *
 ****************************/

template <class F, class M> struct isSparseMatrix : public std::false_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::CSR>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::CSR_ZO>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::COO>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::COO_ZO>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::ELL>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::ELL_ZO>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::SELL>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::SELL_ZO>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::ELL_simd>> : public std::true_type {};

template <class Field>
struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::ELL_simd_ZO>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::CSR_HYB>> : public std::true_type {};

template <class Field> struct isSparseMatrix<Field, Sparse<Field, SparseMatrix_t::HYB_ZO>> : public std::true_type {};

template <class F, class M> struct isZOSparseMatrix : public std::false_type {};

template <class Field> struct isZOSparseMatrix<Field, Sparse<Field, SparseMatrix_t::CSR_ZO>> : public std::true_type {};

template <class Field> struct isZOSparseMatrix<Field, Sparse<Field, SparseMatrix_t::COO_ZO>> : public std::true_type {};

template <class Field> struct isZOSparseMatrix<Field, Sparse<Field, SparseMatrix_t::ELL_ZO>> : public std::true_type {};

template <class Field>
struct isZOSparseMatrix<Field, Sparse<Field, SparseMatrix_t::SELL_ZO>> : public std::true_type {};

template <class Field>
struct isZOSparseMatrix<Field, Sparse<Field, SparseMatrix_t::ELL_simd_ZO>> : public std::true_type {};

using ZOSparseMatrix = std::true_type;
using NotZOSparseMatrix = std::false_type;

/*********************************************************************************************************************
 *
 *    Sparse Details
 *
 *********************************************************************************************************************/

namespace sparse_details {

struct Stats {
    uint64_t rowdim = 0;
    uint64_t coldim = 0;
    uint64_t nOnes = 0;
    uint64_t nMOnes = 0;
    uint64_t nOthers = 0;
    uint64_t nnz = 0;
    uint64_t maxRow = 0;
    uint64_t minRow = 0;
    uint64_t averageRow = 0;
    uint64_t deviationRow = 0;
    uint64_t maxCol = 0;
    uint64_t minCol = 0;
    uint64_t averageCol = 0;
    uint64_t deviationCol = 0;
    uint64_t minColDifference = 0;
    uint64_t maxColDifference = 0;
    uint64_t averageColDifference = 0;
    uint64_t deviationColDifference = 0;
    uint64_t minRowDifference = 0;
    uint64_t maxRowDifference = 0;
    uint64_t averageRowDifference = 0;
    uint64_t deviationRowDifference = 0;
    uint64_t nDenseRows = 0;
    uint64_t nDenseCols = 0;
    uint64_t nEmptyRows = 0;
    uint64_t nEmptyCols = 0;
    uint64_t nEmptyColsEnd = 0;
    std::vector<uint64_t> rows;
    std::vector<uint64_t> denseRows;
    std::vector<uint64_t> denseCols;
    std::vector<uint64_t> cols;

    void print() {
        std::cout << "Row dimension : " << rowdim << std::endl;
        std::cout << "Col dimension : " << coldim << std::endl;

        std::cout << "Number of nnz : " << nnz << std::endl;
        std::cout << "Number of 1 : " << nOnes << std::endl;
        std::cout << "Number of -1 : " << nMOnes << std::endl;
        std::cout << "Number of others : " << nOthers << std::endl;

        std::cout << "Max number of nnz in a row : " << maxRow << std::endl;
        std::cout << "Min number of nnz in a row : " << minRow << std::endl;
        std::cout << "Average number of nnz in a row : " << averageRow << std::endl;
        std::cout << "Deviation number of nnz in a row : " << deviationRow << std::endl;

        std::cout << "Max number of nnz in a col : " << maxCol << std::endl;
        std::cout << "Min number of nnz in a col : " << minCol << std::endl;
        std::cout << "Average number of nnz in a col : " << averageCol << std::endl;
        std::cout << "Deviation number of nnz in a col : " << deviationCol << std::endl;

        std::cout << "Number of empty rows : " << nEmptyRows << std::endl;
        std::cout << "Number of empty cols : " << nEmptyCols << std::endl;
        std::cout << "Number of empty cols end : " << nEmptyColsEnd << std::endl;
    }
};

template <class It> double computeDeviation(It begin, It end);

template <class Field>
Stats getStat(const Field &F, const index_t *row, const index_t *col, typename Field::ConstElement_ptr val,
              uint64_t rowdim, uint64_t coldim, uint64_t nnz);

template <class Field>
inline void init_y(const Field &F, const size_t m, const typename Field::Element b, typename Field::Element_ptr y,
                   FieldCategories::ModularTag);

template <class Field>
inline void init_y(const Field &F, const size_t m, const typename Field::Element b, typename Field::Element_ptr y,
                   FieldCategories::UnparametricTag);

template <class Field>
inline void init_y(const Field &F, const size_t m, const typename Field::Element b, typename Field::Element_ptr y,
                   FieldCategories::GenericTag);

template <class Field>
inline void init_y(const Field &F, const size_t m, const size_t n, const typename Field::Element b,
                   typename Field::Element_ptr y, const int ldy, FieldCategories::UnparametricTag);

template <class Field>
inline void init_y(const Field &F, const size_t m, const size_t n, const typename Field::Element b,
                   typename Field::Element_ptr y, const int ldy, FieldCategories::GenericTag);

template <class Field>
inline void init_y(const Field &F, const size_t m, const size_t n, const typename Field::Element b,
                   typename Field::Element_ptr y, const int ldy, FieldCategories::ModularTag);

/*************************************
        fspmv
**************************************/
template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag, std::false_type);

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::UnparametricTag, std::false_type);

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::ModularTag, std::false_type);

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag, std::true_type);

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::UnparametricTag, std::true_type);

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::ModularTag, std::true_type);

/*************************************
        pfspmv
**************************************/
template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::GenericTag, std::false_type);

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::UnparametricTag, std::false_type);

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::ModularTag, std::false_type);

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::GenericTag, std::true_type);

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::UnparametricTag, std::true_type);

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::ModularTag, std::true_type);

/*************************************
        fspmm
**************************************/
template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag, std::false_type);

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::UnparametricTag, std::false_type);

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::ModularTag, std::false_type);

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag, std::true_type);

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::UnparametricTag, std::true_type);

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::ModularTag, std::true_type);
}

/*********************************************************************************************************************
 *
 *    SpMV, SpMM, pSpMV, pSpMM
 *
 *********************************************************************************************************************/

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, const typename Field::Element &beta,
                  typename Field::Element_ptr y);

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, const typename Field::Element &beta,
                   typename Field::Element_ptr y);

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, const typename Field::Element &beta,
                  typename Field::Element_ptr y);

template <class Field, class SM>
inline void pfspmm(const Field &F, const SM &A, typename Field::ConstElement_ptr x, const typename Field::Element &beta,
                   typename Field::Element_ptr y);
}

#include "fflas-ffpack/fflas/fflas_sparse.inl"

#endif // __FFLASFFPACK_fflas_fflas_sparse_H