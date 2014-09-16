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

#ifndef __FFLASFFPACK_fflas_ffpack_utils_simd256_INL
#define __FFLASFFPACK_fflas_ffpack_utils_simd256_INL

template<class T>
struct Simd256;

template<>
struct Simd256<double>
{
#if defined(__FFLASFFPACK_USE_AVX) or defined(__FFLASFFPACK_USE_AVX2)
	using vect_t = __m256d;

	static const constexpr size_t vect_size = 4;

	static const constexpr size_t alignment = 32;

	static INLINE CONST vect_t zero() {return _mm256_setzero_pd();}

	static INLINE CONST vect_t set1(const double x) {return _mm256_set1_pd(x);}

	static INLINE CONST vect_t set(const double x1, const double x2, const double x3, const double x4) {return _mm256_set_pd(x4, x3, x2, x1);}

	template<class T>
	static INLINE PURE vect_t gather(const double * const p, const T * const idx) {
		// TODO AVX2 Gather
		return _mm256_set_pd(p[idx[3]], p[idx[2]], p[idx[1]], p[idx[0]]);
	}

	static INLINE PURE vect_t load(const double * const p) {return _mm256_load_pd(p);}

	static INLINE PURE vect_t loadu(const double * const p) {return _mm256_loadu_pd(p);}

	static INLINE void store(const double * p, vect_t v) {_mm256_store_pd(const_cast<double*>(p), v);}

	static INLINE CONST vect_t add(vect_t a, vect_t b) {return _mm256_add_pd(a, b);}

	static INLINE CONST vect_t sub(const vect_t a, const vect_t b) {return _mm256_sub_pd(a, b);}

	static INLINE CONST vect_t mul(const vect_t a, const vect_t b) {return _mm256_mul_pd(a, b);}

	static INLINE CONST vect_t madd(const vect_t c, const vect_t a, const vect_t b) {
#ifdef __FMA__
		return _mm256_fmadd_pd(a, b, c);
#else
		return _mm256_add_pd(c, _mm256_mul_pd(a, b));
#endif
	}

	static INLINE CONST vect_t nmadd(const vect_t c, const vect_t a, const vect_t b) {
#ifdef __FMA__
		return _mm256_fnmadd_pd(a, b, c);
#else
		return _mm256_sub_pd(c, _mm256_mul_pd(a, b));
#endif
	}

	static INLINE CONST vect_t msub(const vect_t c, const vect_t a, const vect_t b) {
#ifdef __FMA__
		return _mm256_fmsub_pd(a, b, c);
#else
		return _mm256_sub_pd(_mm256_mul_pd(a, b), c);
#endif
	}

	static INLINE CONST vect_t eq(const vect_t a, const vect_t b) {return _mm256_cmp_pd(a, b, _CMP_EQ_OQ);}

	static INLINE CONST vect_t lesser(const vect_t a, const vect_t b) {return _mm256_cmp_pd(a, b, _CMP_LT_OS);}

	static INLINE CONST vect_t lesser_eq(const vect_t a, const vect_t b) {return _mm256_cmp_pd(a, b, _CMP_LE_OS);}

	static INLINE CONST vect_t greater(const vect_t a, const vect_t b) {return _mm256_cmp_pd(a, b, _CMP_GT_OS);}

	static INLINE CONST vect_t greater_eq(const vect_t a, const vect_t b) {return _mm256_cmp_pd(a, b, _CMP_GE_OS);}

	static INLINE CONST vect_t vand(const vect_t a, const vect_t b) {return _mm256_and_pd(a, b);}

	static INLINE CONST vect_t vor(const vect_t a, const vect_t b) {return _mm256_or_pd(a, b);}

	static INLINE CONST vect_t floor(const vect_t a) {return _mm256_floor_pd(a);}

	static INLINE CONST vect_t round(const vect_t a) {return _mm256_round_pd(a, _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC);}

	static INLINE CONST vect_t hadd(const vect_t a, const vect_t b) {return _mm256_hadd_pd(a, b);}

	static INLINE CONST double hadd_to_scal(const vect_t a) {
		return ((const double*)&a)[0] + ((const double*)&a)[1] + ((const double*)&a)[2] + ((const double*)&a)[3];
	}

#else // __AVX__
#error "You need AVX instructions to perform 256bits operations on double"
#endif
};

template<>
struct Simd256<float>
{
#if defined(__FFLASFFPACK_USE_AVX) or defined(__FFLASFFPACK_USE_AVX2)
	using vect_t = __m256;

	static const constexpr size_t vect_size = 8;

	static const constexpr size_t alignment = 32;

	static INLINE CONST vect_t zero() {return _mm256_setzero_ps();}

	static INLINE CONST vect_t set1(const float x) {return _mm256_set1_ps(x);}

	static INLINE CONST vect_t set(const float x1, const float x2, const float x3, const float x4, const float x5, const float x6, const float x7, const float x8) {return _mm256_set_ps(x8, x7, x6, x5, x4, x3, x2, x1);}

	template<class T>
	static INLINE PURE vect_t gather(const float * const p, const T * const idx) {
		// TODO AVX2 Gather
		return _mm256_set_ps(p[idx[7]], p[idx[6]], p[idx[5]], p[idx[4]], p[idx[3]], p[idx[2]], p[idx[1]], p[idx[0]]);
	}

	static INLINE PURE vect_t load(const float * const p) {return _mm256_load_ps(p);}

	static INLINE PURE vect_t loadu(const float * const p) {return _mm256_loadu_ps(p);}

	static INLINE void store(const float * p, const vect_t v) {_mm256_store_ps(const_cast<float*>(p), v);}

	static INLINE CONST vect_t add(const vect_t a, const vect_t b) {return _mm256_add_ps(a, b);}

	static INLINE CONST vect_t sub(const vect_t a, const vect_t b) {return _mm256_sub_ps(a, b);}

	static INLINE CONST vect_t mul(const vect_t a, const vect_t b) {return _mm256_mul_ps(a, b);}

	static INLINE CONST vect_t madd(const vect_t c, const vect_t a, const vect_t b) {
#ifdef __FMA__
		return _mm256_fmadd_ps(a, b, c);
#else
		return _mm256_add_ps(c, _mm256_mul_ps(a, b));
#endif
	}

	static INLINE CONST vect_t nmadd(const vect_t c, const vect_t a, const vect_t b) {
#ifdef __FMA__
		return _mm256_fnmadd_ps(a, b, c);
#else
		return _mm256_sub_ps(c, _mm256_mul_ps(a, b));
#endif
	}

	static INLINE CONST vect_t msub(const vect_t c, const vect_t a, const vect_t b) {
#ifdef __FMA__
		return _mm256_fmsub_ps(a, b, c);
#else
		return _mm256_sub_ps(_mm256_mul_ps(a, b), c);
#endif
	}

	static INLINE CONST vect_t eq(const vect_t a, const vect_t b) {return _mm256_cmp_ps(a, b, _CMP_EQ_OQ);}

	static INLINE CONST vect_t lesser(const vect_t a, const vect_t b) {return _mm256_cmp_ps(a, b, _CMP_LT_OS);}

	static INLINE CONST vect_t lesser_eq(const vect_t a, const vect_t b) {return _mm256_cmp_ps(a, b, _CMP_LE_OS);}

	static INLINE CONST vect_t greater(const vect_t a, const vect_t b) {return _mm256_cmp_ps(a, b, _CMP_GT_OS);}

	static INLINE CONST vect_t greater_eq(const vect_t a, const vect_t b) {return _mm256_cmp_ps(a, b, _CMP_GE_OS);}

	static INLINE CONST vect_t vand(const vect_t a, const vect_t b) {return _mm256_and_ps(a, b);}

	static INLINE CONST vect_t vor(const vect_t a, const vect_t b) {return _mm256_or_ps(a, b);}

	static INLINE CONST vect_t floor(const vect_t a) {return _mm256_floor_ps(a);}

	static INLINE CONST vect_t round(const vect_t a) {return _mm256_round_ps(a, _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC);}

	static INLINE CONST vect_t hadd(const vect_t a, const vect_t b) {return _mm256_hadd_ps(a, b);}

	static INLINE CONST float hadd_to_scal(const vect_t a) {
		return ((const float*)&a)[0] + ((const float*)&a)[1] + ((const float*)&a)[2] + ((const float*)&a)[3] + ((const float*)&a)[4] + ((const float*)&a)[5] + ((const float*)&a)[6] + ((const float*)&a)[7];
	}

#else // __AVX__
#error "You need AVX instructions to perform 256bits operations on float"
#endif
};

template<>
struct Simd256<int64_t>
{
#if defined(__FFLASFFPACK_USE_AVX) or defined(__FFLASFFPACK_USE_AVX2)
	using vect_t = __m256i;
	using half_t = __m128i;

	static const constexpr size_t vect_size = 2;

	static const constexpr size_t alignment = 16;

	static INLINE PURE vect_t load(const int64_t * const p) {return _mm256_load_si256(reinterpret_cast<const vect_t*>(p));}

	static INLINE PURE vect_t loadu(const int64_t * const p) {return _mm256_loadu_si256(reinterpret_cast<const vect_t*>(p));}

	static INLINE PURE half_t load_half(const int64_t * const p) {return _mm_load_si128(reinterpret_cast<const half_t*>(p));}

	static INLINE PURE half_t loadu_half(const int64_t * const p) {return _mm_loadu_si128(reinterpret_cast<const half_t*>(p));}

	static INLINE void store(const int64_t * p, vect_t v) {_mm256_store_si256(reinterpret_cast<vect_t*>(const_cast<int64_t*>(p)), v);}

	static INLINE void storeu(const int64_t * p, vect_t v) {_mm256_storeu_si256(reinterpret_cast<vect_t*>(const_cast<int64_t*>(p)), v);}

	static INLINE void store_half(const int64_t * p, half_t v) {_mm_store_si128(reinterpret_cast<half_t*>(const_cast<int64_t*>(p)), v);}

	static INLINE void storeu_half(const int64_t * p, half_t v) {_mm_storeu_si128(reinterpret_cast<half_t*>(const_cast<int64_t*>(p)), v);}

	static INLINE CONST vect_t set1(const int64_t x) {return _mm256_set1_epi64x(x);} // actually set2

	static INLINE CONST vect_t add(vect_t a, vect_t b) {
#ifdef __AVX2__
		return _mm256_add_epi64(a, b);
#else
		half_t aa = _mm256_extractf128_si256(a,1);
		half_t ab = _mm256_extractf128_si256(a,2);
		half_t ba = _mm256_extractf128_si256(b,1);
		half_t bb = _mm256_extractf128_si256(b,2);
		vect_t res ;
		_mm256_insertf128_si256(res,_mm_add_epi64(aa,ab),0);
		_mm256_insertf128_si256(res,_mm_add_epi64(ba,bb),1);
		return res ;
#endif
	}

	static INLINE CONST vect_t mul(vect_t a, vect_t b) {
#ifdef __AVX2__
		return _mm256_mul_epi32(a, b);
#else
		half_t aa = _mm256_extractf128_si256(a,1);
		half_t ab = _mm256_extractf128_si256(a,2);
		half_t ba = _mm256_extractf128_si256(b,1);
		half_t bb = _mm256_extractf128_si256(b,2);
		vect_t res ;
		_mm256_insertf128_si256(res,_mm_mul_epi32(aa,ab),0);
		_mm256_insertf128_si256(res,_mm_mul_epi32(ba,bb),1);
		return res ;


#endif
	}

	static INLINE CONST vect_t madd(const vect_t c, vect_t a, vect_t b) {
#ifdef __AVX2__
		return _mm256_add_epi64(c, __mm256_mul_epi32(a,b));
#else
		return add(mul(a,b),c);
#endif
	}

	static INLINE CONST vect_t zero() {return _mm256_setzero_si256();}
#endif

};


#endif // __FFLASFFPACK_fflas_ffpack_utils_simd256_INL
