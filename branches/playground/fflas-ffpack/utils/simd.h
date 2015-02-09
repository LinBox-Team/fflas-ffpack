/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 the FFLAS-FFPACK group
 *
 * Written by   Bastien Vialla<bastien.vialla@lirmm.fr>
 * BB <bbboyer@ncsu.edu>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

#ifndef __FFLASFFPACK_utils_simd_H
#define __FFLASFFPACK_utils_simd_H

#ifdef __FFLASFFPACK_HAVE_CXX11

#include <immintrin.h>

#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define INLINE __attribute__((always_inline)) inline
#else
#define INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define CONST __attribute__((const))
#else
#define CONST
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define PURE __attribute__((pure))
#else
#define PURE
#endif

template<class T>
 struct simdToType;

/*
 * is_simd trait
 */

template<class T>
 struct is_simd
 {
    static const constexpr bool value = false;
    using type = std::integral_constant<bool, false>;
 };

// SSE
#if defined(__FFLASFFPACK_USE_SSE)
#include "fflas-ffpack/utils/simd128.inl"

 template<>
 struct simdToType<__m128d>
 {
    using type = double;
 };

template<>
 struct simdToType<__m128>
 {
    using type = float;
 };

template<>
 struct is_simd<__m128d>
 {
     static const constexpr bool value = true;
     using type = std::integral_constant<bool, true>;
 };

template<>
 struct is_simd<__m128>
 {
     static const constexpr bool value = true;
     using type = std::integral_constant<bool, true>;
 };

#endif // SSE

// AVX
#if defined(__FFLASFFPACK_USE_AVX) or defined(__FFLASFFPACK_USE_AVX2)
#include "fflas-ffpack/utils/simd256.inl"

 template<>
 struct simdToType<__m256d>
 {
    using type = double;
 };

template<>
 struct simdToType<__m256>
 {
    using type = float;
 };

 template<>
 struct is_simd<__m256d>
 {
     static const constexpr bool value = true;
     using type = std::integral_constant<bool, true>;
 };

template<>
 struct is_simd<__m256>
 {
     static const constexpr bool value = true;
     using type = std::integral_constant<bool, true>;
 };

#endif // AVX

/*
 * Simd functors
 */

#if defined(__FFLASFFPACK_USE_AVX) or defined(__FFLASFFPACK_USE_AVX2)

template<class T>
using Simd = Simd256<T>;

// template<class T>
// class Simd{
// using self_t = Simd256<T>;

// using self_t = Simd128<T>;
// };

#elif defined(__FFLASFFPACK_USE_SSE)

template<class T>
using Simd = Simd128<T>;

#endif

#else /* C++11 */
#error "You need a c++11 compiler."
#endif /* c++11 */


#undef INLINE
#undef PURE
#undef CONST

#endif /* __FFLASFFPACK_utils_simd_H */