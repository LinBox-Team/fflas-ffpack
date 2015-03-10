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

/** @file fflas/fflas_sparse.inl
*/

#ifndef __FFLASFFPACK_fflas_fflas_sparse_INL
#define __FFLASFFPACK_fflas_fflas_sparse_INL

namespace FFLAS {
namespace sparse_details {
template <class Field>
inline void init_y(const Field &F, const size_t m, const typename Field::Element b, typename Field::Element_ptr y) {
    if (!F.isOne(b)) {
        if (F.isZero(b)) {
            fzero(F, m, y, 1);
        } else if (F.isMOne(b)) {
            fnegin(F, m, y, 1);
        } else {
            fscalin(F, m, b, y, 1);
        }
    }
}

template <class Field>
inline void init_y(const Field &F, const size_t m, const size_t n, const typename Field::Element b,
                   typename Field::Element_ptr y, const int ldy) {
    if (!F.isOne(b)) {
        if (F.isZero(b)) {
            fzero(F, m, n, y, ldy);
        } else if (F.isMOne(b)) {
            fnegin(F, m, n, y, 1);
        } else {
            fscalin(F, m, n, b, y, 1);
        }
    }
}

} // sparse_details

namespace sparse_details {

template <class Field, class SM, class FC, class MZO>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::MultiPrecisionTag ,FC fc, MZO mzo) {
    sparse_details::fspmv(F, A, x, y, FieldCategories::GenericTag(), MZO());
}

template <class Field, class SM, class FC, class MZO>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag ,FC fc, MZO mzo) {
    sparse_details::fspmv(F, A, x, y, FC(), MZO());
}

// non ZO matrix
template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag, std::false_type) {
    sparse_details_impl::fspmv(F, A, x, y, FieldCategories::GenericTag());
}

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::UnparametricTag, std::false_type) {
    sparse_details_impl::fspmv(F, A, x, y, FieldCategories::UnparametricTag());
}

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::ModularTag, std::false_type) {
    if (A.delayed) {
        sparse_details::fspmv(F, A, x, y, FieldCategories::UnparametricTag(), std::false_type());
        freduce(F, A.m, y, 1);
    } else {
        sparse_details_impl::fspmv(F, A, x, y, A.kmax);
    }
}

// ZO matrix
template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag, std::true_type) {
    if (A.cst == 1) {
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::GenericTag());
    } else if (A.cst == -1) {
        sparse_details_impl::fspmv_mone(F, A, x, y, FieldCategories::GenericTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::GenericTag());
        fflas_delete(x1);
    }
}

template <class Field>
inline void fspmv(const Field &F, const Sparse<Field, SparseMatrix_t::SELL> &A, typename Field::ConstElement_ptr x,
                  typename Field::Element_ptr y, FieldCategories::UnparametricTag, std::true_type) {
#ifdef __FFLASFFPACK_USE_SIMD
    if (A.cst == 1) {
        sparse_details_impl::fspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::fspmv_mone_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::fspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#else
    if (A.cst == 1) {
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::fspmv_mone(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#endif // SIMD
}

template <class Field>
inline void fspmv(const Field &F, const Sparse<Field, SparseMatrix_t::ELL_simd> &A, typename Field::ConstElement_ptr x,
                  typename Field::Element_ptr y, FieldCategories::UnparametricTag, std::true_type) {
#ifdef __FFLASFFPACK_USE_SIMD
    if (A.cst == 1) {
        sparse_details_impl::fspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::fspmv_mone_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::fspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#else
    if (A.cst == 1) {
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::fspmv_mone(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#endif // SIMD
}

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::UnparametricTag, std::true_type) {
    if (A.cst == 1) {
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::fspmv_mone(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::fspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
}

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::ModularTag, std::true_type) {
    sparse_details::fspmv<Field, SM>(F, A, x, y, FieldCategories::UnparametricTag(), std::true_type());
    freduce(F, A.m, y, 1);
}

/***************************** pfspmv ******************************/

#if defined(__FFLASFFPACK_HAVE_OPENMP)

template <class Field, class SM, class FC, class MZO>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::MultiPrecisionTag ,FC fc, MZO mzo) {
    sparse_details::pfspmv(F, A, x, y, FieldCategories::GenericTag(), MZO());
}

template <class Field, class SM, class FC, class MZO>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                  FieldCategories::GenericTag ,FC fc, MZO mzo) {
    sparse_details::pfspmv(F, A, x, y, FC(), MZO());
}

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::GenericTag, std::false_type) {
    sparse_details_impl::pfspmv(F, A, x, y, FieldCategories::GenericTag());
}

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::UnparametricTag, std::false_type) {
    sparse_details_impl::pfspmv(F, A, x, y, FieldCategories::UnparametricTag());
}

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::ModularTag, std::false_type) {
    if (A.delayed) {
        sparse_details::pfspmv(F, A, x, y, FieldCategories::UnparametricTag(), std::false_type());
        freduce(F, A.m, y, 1);
    } else {
        sparse_details_impl::pfspmv(F, A, x, y, A.kmax);
    }
}

// ZO matrix
template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::GenericTag, std::true_type) {
    if (A.cst == 1) {
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::GenericTag());
    } else if (A.cst == -1) {
        sparse_details_impl::pfspmv_mone(F, A, x, y, FieldCategories::GenericTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::GenericTag());
        fflas_delete(x1);
    }
}

template <class Field>
inline void pfspmv(const Field &F, const Sparse<Field, SparseMatrix_t::SELL> &A, typename Field::ConstElement_ptr x,
                   typename Field::Element_ptr y, FieldCategories::UnparametricTag, std::true_type) {
#ifdef __FFLASFFPACK_USE_SIMD
    if (A.cst == 1) {
        sparse_details_impl::pfspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::pfspmv_mone_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::pfspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#else
    if (A.cst == 1) {
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::pfspmv_mone(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#endif // SIMD
}

template <class Field>
inline void pfspmv(const Field &F, const Sparse<Field, SparseMatrix_t::ELL_simd> &A, typename Field::ConstElement_ptr x,
                   typename Field::Element_ptr y, FieldCategories::UnparametricTag, std::true_type) {
#ifdef __FFLASFFPACK_USE_SIMD
    if (A.cst == 1) {
        sparse_details_impl::pfspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::pfspmv_mone_simd(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::pfspmv_one_simd(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#else
    if (A.cst == 1) {
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::pfspmv_mone(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#endif // SIMD
}

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::UnparametricTag, std::true_type) {
    if (A.cst == 1) {
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
    } else if (A.cst == -1) {
        sparse_details_impl::pfspmv_mone(F, A, x, y, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.n, 1, Alignment::CACHE_LINE);
        fscal(F, A.n, A.cst, x, 1, x1, 1);
        sparse_details_impl::pfspmv_one(F, A, x, y, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
}

template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, typename Field::Element_ptr y,
                   FieldCategories::ModularTag, std::true_type) {
    sparse_details::pfspmv<Field, SM>(F, A, x, y, FieldCategories::UnparametricTag(), std::true_type());
    freduce(F, A.m, y, 1);
}
#endif

// /***************************** fspmm *****************************/

template<class Field, class SM, class FCat, class MZO>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::MultiPrecisionTag, FCat, MZO) {
    // std::cout << "fspmm multi" << std::endl;
    sparse_details::fspmm(F, A, blockSize, x, ldx, y, ldy, typename FieldCategories::GenericTag(), MZO());
}

template<class Field, class SM, class FCat, class MZO>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::GenericTag, FCat, MZO) {
    // std::cout << "fspmm not multi" << std::endl;
    sparse_details::fspmm(F, A, blockSize, x, ldx, y, ldy, FCat(), MZO());
}

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::GenericTag, std::false_type) {
    // std::cout << "no ZO Generic" << std::endl;
    sparse_details_impl::fspmm(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
}

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::UnparametricTag, std::false_type) {
// std::cout << "no ZO Unparametric" << std::endl;
#ifdef __FFLASFFPACK_USE_SIMD
    using simd = Simd<typename Field::Element>;
    if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
        (blockSize % simd::vect_size == 0)) {
        // std::cout << "no ZO Unparametric algined" << std::endl;
        sparse_details_impl::fspmm_simd_aligned(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
    }else{
        sparse_details_impl::fspmm_simd_unaligned(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
    }
#else
    sparse_details_impl::fspmm(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
#endif
}

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::ModularTag, std::false_type) {
    // std::cout << "no ZO Modular" << std::endl;
    if (A.delayed) {
        sparse_details::fspmm(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag(),
                              typename std::false_type());
        freduce(F, A.m, blockSize, y, ldy);
    } else {
#ifdef __FFLASFFPACK_USE_SIMD
            using simd = Simd<typename Field::Element>;
            if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
                (blockSize % simd::vect_size == 0)) {
                sparse_details_impl::fspmm_simd_aligned(F, A, blockSize, x, ldx, y, ldy, A.kmax);
            } else {
                sparse_details_impl::fspmm_simd_unaligned(F, A, blockSize, x, ldx, y, ldy, A.kmax);
            }
#else
        sparse_details_impl::fspmm(F, A, blockSize, x, ldx, y, ldy, A.kmax);
#endif
    }
}

// ZO matrix
template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::GenericTag, std::true_type) {
    // std::cout << "ZO Generic" << std::endl;
    if (F.isOne(A.cst)) {
        sparse_details_impl::fspmm_one(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
    } else if (F.isMOne(A.cst)) {
        sparse_details_impl::fspmm_mone(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
    } else {
        auto x1 = fflas_new(F, A.m, blockSize, Alignment::CACHE_LINE);
        fscal(F, A.m, blockSize, A.cst, x, ldx, x1, 1);
        sparse_details_impl::fspmm_one(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
        fflas_delete(x1);
    }
}

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::UnparametricTag, std::true_type) {
// std::cout << "ZO Unparametric" << std::endl;
#ifdef __FFLASFFPACK_USE_SIMD
    using simd = Simd<typename Field::Element>;
    if (F.isOne(A.cst)) {
        if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
            (blockSize % simd::vect_size == 0)) {
            // std::cout << "ZO Unparametric aligned" << std::endl;
            sparse_details_impl::fspmm_one_simd_aligned(F, A, blockSize, x, ldx, y, ldy,
                                                        FieldCategories::UnparametricTag());
        } else {
            // std::cout << "ZO Unparametric unaligned" << std::endl;
            sparse_details_impl::fspmm_one_simd_unaligned(F, A, blockSize, x, ldx, y, ldy,
                                                          FieldCategories::UnparametricTag());
        }
    } else if (F.isMOne(A.cst)) {
        if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
            (blockSize % simd::vect_size == 0)) {
            sparse_details_impl::fspmm_mone_simd_aligned(F, A, blockSize, x, ldx, y, ldy,
                                                         FieldCategories::UnparametricTag());
        } else {
            sparse_details_impl::fspmm_mone_simd_unaligned(F, A, blockSize, x, ldx, y, ldy,
                                                           FieldCategories::UnparametricTag());
        }
    } else {
        auto x1 = fflas_new(F, A.m, blockSize, Alignment::CACHE_LINE);
        fscal(F, A.m, blockSize, A.cst, x, ldx, x1, 1);
        if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
            (blockSize % simd::vect_size == 0)) {
            sparse_details_impl::fspmm_one_simd_aligned(F, A, blockSize, x, ldx, y, ldy,
                                                        FieldCategories::UnparametricTag());
        } else {
            sparse_details_impl::fspmm_one_simd_unaligned(F, A, blockSize, x, ldx, y, ldy,
                                                          FieldCategories::UnparametricTag());
        }
        fflas_delete(x1);
    }
#else
    if (F.isOne(A.cst)) {
        sparse_details_impl::fspmm_one(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
    } else if (F.isMOne(A.cst)) {
        sparse_details_impl::fspmm_mone(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
    } else {
        auto x1 = fflas_new(F, A.m, blockSize, Alignment::CACHE_LINE);
        fscal(F, A.m, blockSize, A.cst, x, ldx, x1, 1);
        sparse_details_impl::fspmm_one(F, A, blockSize, x1, ldx, y, ldy, FieldCategories::UnparametricTag());
        fflas_delete(x1);
    }
#endif
}

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  typename Field::Element_ptr y, int ldy, FieldCategories::ModularTag, std::true_type) {
    // std::cout << "ZO Modular" << std::endl;
    if (A.delayed) {
        sparse_details::fspmm(F, A, blockSize, x, ldx, y, ldy, typename FieldCategories::UnparametricTag(),
                              typename std::true_type());
        freduce(F, blockSize, A.m, y, ldy);
    } else {
        sparse_details_impl::fspmm(F, A, blockSize, x, ldx, y, ldy, A.kmax);
    }
}

// /***************************** pfspmm *****************************/

// template<class Field, class SM, class FCat, class MZO>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::MultiPrecisionTag, FCat fc, MZO mz) {
//     sparse_details::pfspmm(F, A, blockSize, x, ldx, y, ldy, typename FieldCategories::GenericTag(), MZO());
// }

// template<class Field, class SM, class FCat, class MZO>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::GenericTag, FCat fc, MZO mz) {
//     sparse_details::pfspmm(F, A, blockSize, x, ldx, y, ldy, FCat(), MZO());
// }

// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::GenericTag, std::false_type) {
//     // std::cout << "no ZO Generic" << std::endl;
//     /*sparse_details_impl::*/pfspmm(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
// }

// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::UnparametricTag, std::false_type) {
// // std::cout << "no ZO Unparametric" << std::endl;
// #ifdef __FFLASFFPACK_USE_SIMD
//     using simd = Simd<typename Field::Element>;
//     if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
//         (blockSize % simd::vect_size == 0)) {
//         // std::cout << "no ZO Unparametric algined" << std::endl;
//         sparse_details_impl::pfspmm_simd_aligned(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
//     }
// else{
//     sparse_details_impl::fspmm_simd_unaligned(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
// }
// #else
//     sparse_details_impl::pfspmm(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
// #endif
// }

// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::ModularTag, std::false_type) {
//     // std::cout << "no ZO Modular" << std::endl;
//     if (A.delayed) {
//         sparse_details::pfspmm(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag(),
//                               typename std::false_type());
//         freduce(F, A.m, blockSize, y, ldy);
//     } else {
// #ifdef __FFLASFFPACK_USE_SIMD
//         using simd = Simd<typename Field::Element>;
//         if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
//             (blockSize % simd::vect_size == 0)) {
//             sparse_details_impl::pfspmm_simd_aligned(F, A, blockSize, x, ldx, y, ldy, A.kmax);
//         } else {
//             sparse_details_impl::pfspmm_simd_unaligned(F, A, blockSize, x, ldx, y, ldy, A.kmax);
//         }
// #else
//         sparse_details_impl::pfspmm(F, A, blockSize, x, ldx, y, ldy, A.kmax);
// #endif
//     }
// }

// // ZO matrix
// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::GenericTag, std::true_type) {
//     // std::cout << "ZO Generic" << std::endl;
//     if (F.isOne(A.cst)) {
//         sparse_details_impl::pfspmm_one(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
//     } else if (F.isMOne(A.cst)) {
//         sparse_details_impl::pfspmm_mone(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
//     } else {
//         auto x1 = fflas_new(F, A.m, blockSize, Alignment::CACHE_LINE);
//         fscal(F, A.m, blockSize, A.cst, x, ldx, x1, 1);
//         sparse_details_impl::pfspmm_one(F, A, blockSize, x, ldx, y, ldy, FieldCategories::GenericTag());
//         fflas_delete(x1);
//     }
// }

// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::UnparametricTag, std::true_type) {
// // std::cout << "ZO Unparametric" << std::endl;
// #ifdef __FFLASFFPACK_USE_SIMD
//     using simd = Simd<typename Field::Element>;
//     if (F.isOne(A.cst)) {
//         if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
//             (blockSize % simd::vect_size == 0)) {
//             // std::cout << "ZO Unparametric aligned" << std::endl;
//             sparse_details_impl::pfspmm_one_simd_aligned(F, A, blockSize, x, ldx, y, ldy,
//                                                         FieldCategories::UnparametricTag());
//         } else {
//             // std::cout << "ZO Unparametric unaligned" << std::endl;
//             sparse_details_impl::pfspmm_one_simd_unaligned(F, A, blockSize, x, ldx, y, ldy,
//                                                           FieldCategories::UnparametricTag());
//         }
//     } else if (F.isMOne(A.cst)) {
//         if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
//             (blockSize % simd::vect_size == 0)) {
//             sparse_details_impl::pfspmm_mone_simd_aligned(F, A, blockSize, x, ldx, y, ldy,
//                                                          FieldCategories::UnparametricTag());
//         } else {
//             sparse_details_impl::pfspmm_mone_simd_unaligned(F, A, blockSize, x, ldx, y, ldy,
//                                                            FieldCategories::UnparametricTag());
//         }
//     } else {
//         auto x1 = fflas_new(F, A.m, blockSize, Alignment::CACHE_LINE);
//         fscal(F, A.m, blockSize, A.cst, x, ldx, x1, 1);
//         if (((uint64_t)y % simd::alignment == 0) && ((uint64_t)x % simd::alignment == 0) &&
//             (blockSize % simd::vect_size == 0)) {
//             sparse_details_impl::pfspmm_one_simd_aligned(F, A, blockSize, x, ldx, y, ldy,
//                                                         FieldCategories::UnparametricTag());
//         } else {
//             sparse_details_impl::pfspmm_one_simd_unaligned(F, A, blockSize, x, ldx, y, ldy,
//                                                           FieldCategories::UnparametricTag());
//         }
//         fflas_delete(x1);
//     }
// #else
//     if (F.isOne(A.cst)) {
//         sparse_details_impl::pfspmm_one(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
//     } else if (F.isMOne(A.cst)) {
//         sparse_details_impl::pfspmm_mone(F, A, blockSize, x, ldx, y, ldy, FieldCategories::UnparametricTag());
//     } else {
//         auto x1 = fflas_new(F, A.m, blockSize, Alignment::CACHE_LINE);
//         fscal(F, A.m, blockSize, A.cst, x, ldx, x1, 1);
//         sparse_details_impl::pfspmm_one(F, A, blockSize, x1, ldx, y, ldy, FieldCategories::UnparametricTag());
//         fflas_delete(x1);
//     }
// #endif
// }

// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
//                   typename Field::Element_ptr y, int ldy, FieldCategories::ModularTag, std::true_type) {
//     // std::cout << "ZO Modular" << std::endl;
//     if (A.delayed) {
//         sparse_details::pfspmm(F, A, blockSize, x, ldx, y, ldy, typename FieldCategories::UnparametricTag(),
//                               typename std::true_type());
//         freduce(F, blockSize, A.m, y, ldy);
//     } else {
//         sparse_details_impl::pfspmm(F, A, blockSize, x, ldx, y, ldy, A.kmax);
//     }
// }

} // sparse details

template <class Field, class SM>
inline void fspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, const typename Field::Element &beta,
                  typename Field::Element_ptr y) {
    sparse_details::init_y(F, A.m, beta, y);
    sparse_details::fspmv<Field, SM>(F, A, x, y, typename FieldTraits<Field>::value(), typename FieldTraits<Field>::category(),
                                     typename isZOSparseMatrix<Field, SM>::type());
}

template <class Field, class SM>
inline void fspmm(const Field &F, const SM &A, int blockSize, typename Field::ConstElement_ptr x, int ldx,
                  const typename Field::Element &beta, typename Field::Element_ptr y, int ldy) {
    sparse_details::init_y(F, A.m, blockSize, beta, y, ldy);
    // std::cout << "fspmm" << std::endl;
    sparse_details::fspmm<Field, SM>(F, A, blockSize, x, ldx, y, ldy, typename FieldTraits<Field>::value(),
                 typename FieldTraits<Field>::category(), typename isZOSparseMatrix<Field, SM>::type());
}

#if defined(__FFLASFFPACK_HAVE_OPENMP)
template <class Field, class SM>
inline void pfspmv(const Field &F, const SM &A, typename Field::ConstElement_ptr x, const typename Field::Element &beta,
                   typename Field::Element_ptr y) {
    sparse_details::init_y(F, A.m, beta, y);
    sparse_details::pfspmv<Field, SM>(F, A, x, y, typename FieldTraits<Field>::value(), typename FieldTraits<Field>::category(),
                                      typename isZOSparseMatrix<Field, SM>::type());
}
#endif
// template <class Field, class SM>
// inline void pfspmm(const Field &F, const SM &A, int blockSize,
//                    typename Field::ConstElement_ptr x, int ldx,
//                    const typename Field::Element &beta,
//                    typename Field::Element_ptr y, int ldy) {
//     sparse_details::init_y(F, A.m, blockSize, beta, y, ldy);
//     sparse_details::pfspmm<Field, SM>(
//         F, A, blockSize, x, ldx, y, ldy, typename FieldTraits<Field>::value(),
//         typename FieldTraits<Field>::category(),
//         typename isZOSparseMatrix<Field, SM>::type());
// }
}


#endif // __FFLASFFPACK_fflas_fflas_sparse_INL