/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 the FFLAS-FFPACK group
 *
 * Written by BB <bbboyer@ncsu.edu>
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

/** @file fflas_fgemm/fgemm_winograd.h
 * @brief Strassen--Winograd matrix multiplication.
 * @warning The domain is supposed to be a field since some divisions are required for efficiency purposes
 * An alternative has to be written for finite rings if necessary
 */

#ifndef __FFLASFFPACK_fflas_fflas_fgemm_winograd_INL
#define __FFLASFFPACK_fflas_fflas_fgemm_winograd_INL

#include "fgemm_classical.inl"
#include <stdint.h>


#include "schedule_winograd.inl"
#include "schedule_winograd_acc.inl"
#include "schedule_winograd_acc_ip.inl"
#include "schedule_winograd_ip.inl"
// #include "fflas_fgemm/bini.inl"

#ifndef NEWWINO
#define NEWWINO
#endif

//#define OLDWINO

#include "fflas-ffpack/fflas-ffpack-config.h"


// DynamicPeeling, WinogradCalc
namespace FFLAS { namespace Protected {

	/** \brief Computes the number of recursive levels to perform.
	 *
	 * \param m the common dimension in the product AxB
	 */
	template<class Field>
	inline int WinogradSteps (const Field & F, const size_t & m)
	{
		int w = 0;
		size_t mt = m;
		while ( mt >= WINOTHRESHOLD ) {
			++w;
			mt >>= 1;
		}
		return w;
	}

	template<>
	inline int WinogradSteps (const FFPACK:: Modular<double> & F, const size_t & m)
	{
		int w = 0;
		size_t mt = m;
		while ( mt >= __FFLASFFPACK_WINOTHRESHOLD ) {
			++w;
			mt >>= 1;
		}
		return w;
	}

	template<>
	inline int WinogradSteps (const FFPACK:: ModularBalanced<double> & F, const size_t & m)
	{
		int w = 0;
		size_t mt = m;
		while ( mt >= __FFLASFFPACK_WINOTHRESHOLD_BAL ) {
			++w;
			mt >>= 1;
		}
		return w;
	}

	template<>
	inline int WinogradSteps (const FFPACK:: Modular<float> & F, const size_t & m)
	{
		int w = 0;
		size_t mt = m;
		while ( mt >= __FFLASFFPACK_WINOTHRESHOLD_FLT ) {
			++w;
			mt >>= 1;
		}
		return w;
	}

	template<>
	inline int WinogradSteps (const FFPACK:: ModularBalanced<float> & F, const size_t & m)
	{
		int w = 0;
		size_t mt = m;
		while ( mt >= __FFLASFFPACK_WINOTHRESHOLD_BAL_FLT ) {
			++w;
			mt >>= 1;
		}
		return w;
	}

	template  < class Field, class FieldTrait >
	inline void
	DynamicPeeling (const Field& F,
			const FFLAS_TRANSPOSE ta,
			const FFLAS_TRANSPOSE tb,
			const size_t m, const size_t n, const size_t k,
			const size_t mr, const size_t nr, const size_t kr,
			const typename Field::Element alpha,
			typename Field::Element_ptr A, const size_t lda,
			typename Field::Element_ptr B, const size_t ldb,
			const typename Field::Element beta,
			typename Field::Element_ptr C, const size_t ldc,
			MMHelper<Field, MMHelperAlgo::Winograd, FieldTrait> & H,
			const double Cmin, const double Cmax)
	{
		typename Field::Element_ptr a12, a21, b12, b21;
		size_t inca12, inca21, incb12, incb21, ma, na, mb, nb;
		size_t mkn = nr + (kr << 1)+  (mr << 2);

		if (ta == FflasTrans) {
			ma = k; na = m;
			a12 = A+(k-1)*lda; inca12 = 1;
			a21 = A+m-1; inca21 = lda;
		}
		else {
			ma = m;	na = k;
			a12 = A+k-1; inca12 = lda;
			a21 = A+(m-1)*lda; inca21 = 1;
		}
		if (tb == FflasTrans) {
			mb = n;	nb = k;
			b12 = B+(n-1)*ldb; incb12 = 1;
			b21 = B+k-1; incb21 = ldb;
		}
		else {
			mb = k; nb = n;
			b12 = B+n-1; incb12 = ldb;
			b21 = B+(k-1)*ldb; incb21 = 1;
		}
		MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> Hacc(H);
		MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> HModd(H);
		MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> HNodd(H);

		Hacc.Cmin = H.Outmin; Hacc.Cmax = H.Outmax;
		HModd.Cmin = Cmin; HModd.Cmax = Cmax;
		HModd.Amax = H.Bmax; HModd.Amin = H.Bmin;
		HModd.Bmax = H.Amax; HModd.Bmin = H.Amin;
		HNodd.Cmin = Cmin; HNodd.Cmax = Cmax;

		switch (mkn) {
		case 1: // n oddsized
			fgemv (F, ta, ma, na, alpha, A, lda, b12, incb12, beta, C+n-1,ldc, HNodd);
			break;

		case 2: // k oddsized
			fger (F, m, n, alpha, a12, inca12, b21, incb21, C, ldc, Hacc);
			break;

		case 3: // n, k oddsized
			fgemv (F, ta, ma, na, alpha, A, lda, b12, incb12, beta, C+n-1,ldc, HNodd);
			fger (F, m, n-1, alpha, a12, inca12, b21, incb21, C, ldc, Hacc);
			break;

		case 4: // m oddsized
			fgemv(F, (tb == FflasTrans)?FflasNoTrans:FflasTrans, mb, nb,
			      alpha, B, ldb, a21, inca21, beta, C+(m-1)*ldc, 1, HModd);
			break;

		case 5: // m, n oddsized
			if (tb == FflasTrans) mb--;
			else nb--;
			fgemv (F, ta, ma, na, alpha, A, lda, b12, incb12, beta, C+n-1, ldc, HNodd);
			fgemv (F, (tb==FflasTrans)?FflasNoTrans:FflasTrans, mb, nb,
			       alpha, B, ldb, a21, inca21, beta, C+(m-1)*ldc, 1, HModd);
			break;

		case 6: // m, k oddsized
			fger (F, m-1, n, alpha, a12, inca12, b21, incb21, C, ldc, Hacc);
			fgemv(F, (tb==FflasTrans)?FflasNoTrans:FflasTrans, mb, nb,
			      alpha, B, ldb, a21, inca21, beta, C+(m-1)*ldc, 1, HModd);
			break;

		case 7: // m, k, n oddsized
			if (tb == FflasTrans) mb--;
			else nb--;
			// Block NW
			fger (F, m-1, n-1, alpha, a12, inca12, b21, incb21, C, ldc, Hacc);
			// Block SW
			fgemv (F, (tb==FflasTrans)?FflasNoTrans:FflasTrans, mb, nb,
			       alpha, B, ldb, a21, inca21, beta, C+(m-1)*ldc, 1, HModd);
			HModd.checkOut(F, m-1,n-1, C, ldc);
                        // Block E
			fgemv (F, ta, ma, na, alpha, A, lda, b12, incb12, beta, C+n-1, ldc, HNodd);
			break;
		}
		H.Outmin = min4(HModd.Outmin,HNodd.Outmin, Hacc.Outmin, H.Outmin);
		H.Outmax = max4(HModd.Outmax,HNodd.Outmax, Hacc.Outmax, H.Outmax);
		H.checkOut(F, m,n, C, ldc);
	}

		template  < class Field, class FieldTrait >
	inline void
	DynamicPeeling2 (const Field& F,
			 const FFLAS_TRANSPOSE ta,
			 const FFLAS_TRANSPOSE tb,
			 const size_t m, const size_t n, const size_t k,
			 const size_t mr, const size_t nr, const size_t kr,
			 const typename Field::Element alpha,
			 typename Field::Element_ptr A, const size_t lda,
			 typename Field::Element_ptr B, const size_t ldb,
			 const typename Field::Element beta,
			 typename Field::Element_ptr C, const size_t ldc,
			 MMHelper<Field, MMHelperAlgo::Winograd, FieldTrait> & H,
			 const double Cmin, const double Cmax)
	{
		size_t mkn =(size_t)( (bool)(nr > 0)+ ((bool)(kr > 0) << 1)+  ((bool)(mr > 0) << 2));
		if (mkn == 0) return;

		 typename Field::Element_ptr a12, a21, b12, b21;
		if (ta == FflasTrans) {
			a12 = A+(k-kr)*lda;
			a21 = A+(m-mr);
		}
		else {
			a12 = A+(k-kr);
			a21 = A+(m-mr)*lda;
		}
		if (tb == FflasTrans) {
			b12 = B+(n-nr)*ldb;
			b21 = B+(k-kr);
		}
		else {
			b12 = B+(n-nr);
			b21 = B+(k-kr)*ldb;
		}

		MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> Hacc(H);
		MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> HModd(H);
		MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> HNodd(H);

		Hacc.Cmin = H.Outmin; Hacc.Cmax = H.Outmax;
		HModd.Cmin = Cmin; HModd.Cmax = Cmax;
		HModd.Amax = H.Bmax; HModd.Amin = H.Bmin;
		HModd.Bmax = H.Amax; HModd.Bmin = H.Amin;
		HNodd.Cmin = Cmin; HNodd.Cmax = Cmax;

		switch (mkn) {
		case 1: // n oddsized
			fgemm (F, ta, tb, m, nr, k, alpha, A, lda, b12, ldb, beta, C+(n-nr), ldc, HNodd);
			break;

		case 2: // k oddsized
			fgemm (F, ta, tb, m, n, kr, alpha, a12, lda, b21, ldb, F.one, C, ldc, Hacc);
			break;

		case 3: // n, k oddsized
			fgemm (F, ta, tb, m, nr, k, alpha, A, lda, b12, ldb, beta, C+(n-nr), ldc, HNodd);
			fgemm (F, ta, tb, m, n-nr, kr, alpha, a12, lda, b21, ldb, F.one, C, ldc, Hacc);
			break;

		case 4: // m oddsized
			fgemm (F,  ta, tb, mr, n, k, alpha, a21, lda, B, ldb, beta, C+(m-mr)*ldc, ldc, HModd);
			break;

		case 5: // m, n oddsized
			fgemm (F, ta, tb, m, nr, k, alpha, A, lda, b12, ldb, beta, C+(n-nr), ldc, HNodd);
			fgemm (F, ta, tb, mr, n-nr, k, alpha, a21, lda, B, ldb, beta, C+(m-mr)*ldc, ldc, HModd);
			break;

		case 6: // m, k oddsized
			fgemm (F, ta, tb, m-mr, n, kr, alpha, a12, lda, b21, ldb, F.one, C, ldc, Hacc);
			fgemm (F, ta, tb, mr, n, k, alpha, a21, lda, B, ldb, beta, C+(m-mr)*ldc, ldc, HModd);
			break;

		case 7: // m, k, n oddsized
			// Block NW
			fgemm (F, ta, tb, m-mr, n-nr, kr, alpha, a12, lda, b21, ldb, F.one, C, ldc, Hacc);
			// Block SW
			fgemm (F,  ta, tb, mr, n-nr, k, alpha, a21, lda, B, ldb, beta, C+(m-mr)*ldc, ldc, HModd);
			// Block NE
			fgemm (F, ta, tb, m, nr, k, alpha, A, lda, b12, ldb, beta, C+(n-nr), ldc, HNodd);
			break;
		}
		H.Outmin = min4(HModd.Outmin,HNodd.Outmin, Hacc.Outmin, H.Outmin);
		H.Outmax = max4(HModd.Outmax,HNodd.Outmax, Hacc.Outmax, H.Outmax);
		H.checkOut(F, m,n, C, ldc);
	}

	// #define NEWIP
	// #define NEWACCIP

	// Switch between the scheduling for Strassen-Winograd Multiplication
	template < class Field, class FieldTrait >
	inline void WinogradCalc (const Field& F,
				  const FFLAS_TRANSPOSE ta,
				  const FFLAS_TRANSPOSE tb,
				  const size_t mr, const size_t nr, const size_t kr,
				  const typename Field::Element alpha,
				  typename Field::Element_ptr A,const size_t lda,
				  typename Field::Element_ptr B,const size_t ldb,
				  const typename Field::Element beta,
				  typename Field::Element_ptr C, const size_t ldc,
				  MMHelper<Field, MMHelperAlgo::Winograd, FieldTrait> & H)
	{
#if defined(NEWIP) or defined(NEWACCIP)  /*  XXX TESTS ONLY */
		typedef typename Field::Element Element ;
		Element_ptr Ac;
		Element_ptr Bc;
		if (ta == FflasNoTrans) {
			Ac = fflas_new (F, mr*2, lda);
			fcopy(F,mr*2,kr*2,A,lda,Ac,lda);
		}
		else {
			Ac = fflas_new (F, kr*2, lda);
			fcopy(F,kr*2,mr*2,A,lda,Ac,lda);
		}
		if (tb == FflasNoTrans) {
			Bc = fflas_new (F, kr*2, ldb);
			fcopy(F,kr*2,nr*2,B,ldb,Bc,ldb);
		}
		else {
			Bc = fflas_new (F, nr*2, ldb);
			fcopy(F,nr*2,kr*2,B,ldb,Bc,ldb);
		}
#endif

		if (F.isZero(beta)) {
#ifdef NEWIP /*  NOT IP --- TESTS ONLY */
			// (kr == nr  && kr <= mr /*  if not transposed */)
			// we copy because they erase stuff
			// bool normal =  (ta == FflasNoTrans && tb == FflasNoTrans) ;
			bool normal = true;

			// std::cout << (ta==FflasNoTrans) << ',' << (tb==FflasNoTrans) << std::endl;

			if (kr == nr && kr == mr && normal) {
				// BLAS3::Winograd_L_S(F,ta,tb,mr,nr,kr,alpha,Ac,lda,Bc,ldb,beta,C,ldc,H);
				// BLAS3::Winograd_R_S(F,ta,tb,mr,nr,kr,alpha,Ac,lda,Bc,ldb,beta,C,ldc,H);
				BLAS3::Winograd_LR_S(F,ta,tb,mr,nr,kr,alpha,Ac,lda,Bc,ldb,beta,C,ldc,H);
			}
			else
#endif
			{
				BLAS3::Winograd(F,ta,tb,mr,nr,kr,alpha,A,lda,B,ldb,beta,C,ldc,H);
			}

		}
		else {
#ifdef NEWACCIP /*  test only */
			// std::cout << (ta==FflasNoTrans) << ',' << (tb==FflasNoTrans) << std::endl;
			if (kr == nr && kr == mr ) {
				// std::cout << 'h' << std::endl;
				BLAS3::WinogradAcc_L_S(F,ta,tb,mr,nr,kr,alpha,Ac,lda,Bc,ldb,beta,C,ldc,H);
				// BLAS3::WinogradAcc_R_S(F,ta,tb,mr,nr,kr,alpha,Ac,lda,Bc,ldb,beta,C,ldc,H);
			}
			else {
				BLAS3::WinogradAcc_LR(F,ta,tb,mr,nr,kr,alpha,Ac,lda,Bc,ldb,beta,C,ldc,H);
			}


#elif defined(NEWWINO)
			BLAS3::WinogradAcc_3_21(F,ta,tb,mr,nr,kr,alpha,A,lda,B,ldb,beta,C,ldc,H);
#elif defined(OLDWINO)
			BLAS3::WinogradAcc_3_23(F,ta,tb,mr,nr,kr,alpha,A,lda,B,ldb,beta,C,ldc,H);
#elif defined(NEWACC)
			// BLAS3::WinogradAcc_2_24(F,ta,tb,mr,nr,kr,alpha,A,lda,B,ldb,beta,C,ldc,H);
			BLAS3::WinogradAcc_2_27(F,ta,tb,mr,nr,kr,alpha,A,lda,B,ldb,beta,C,ldc,H);
#else
#error "you need to make a choice for a BLAS3 mat mul schedule"
#endif

		}
#if defined(NEWIP) or defined(NEWACCIP)  /*  NOT IP --- TESTS ONLY */
		fflas_delete (Ac);
		fflas_delete (Bc);
#endif

	} // WinogradCalc

//#define OLD_DYNAMIC_PEELING

}// namespace Protected
} // FFLAS


namespace FFLAS{
	template<class Field, class FieldTrait>
	inline  typename Field::Element_ptr
	fgemm (const Field& F,
	       const FFLAS_TRANSPOSE ta,
	       const FFLAS_TRANSPOSE tb,
	       const size_t m, const size_t n, const size_t k,
	       const typename Field::Element alpha,
	       typename Field::Element_ptr A, const size_t lda,
	       typename Field::Element_ptr B, const size_t ldb,
	       const typename Field::Element beta,
	       typename Field::Element_ptr C, const size_t ldc,
	       MMHelper<Field, MMHelperAlgo::Winograd, FieldTrait> & H)
	{
		if (!m || !n ) return C;

		if (!k){
			    //TODO: update helper
			fscalin(F,m,n,beta,C,ldc);
			return C;
		}

		if (H.recLevel == 0){
			MMHelper<Field, MMHelperAlgo::Classic, FieldTrait> HC(H);
			fgemm (F, ta, tb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, HC);
			H.Outmax = HC.Outmax;
			H.Outmin = HC.Outmin;
			return C;
		}

        if (H.recLevel < 0) {
            H.recLevel = Protected::WinogradSteps (F, min3(m,k,n));
        }

		// Then w >0
		double Cmin = H.Cmin;
		double Cmax = H.Cmax;

#ifdef OLD_DYNAMIC_PEELING

		Protected::WinogradCalc (F, ta, tb, m/2, n/2, k/2, alpha, A, lda, B, ldb, beta, C, ldc, H);

		FFLASFFPACK_check(m-(m/2)*2 == (m&0x1));
		FFLASFFPACK_check(n-(n/2)*2 == (n&0x1));
		FFLASFFPACK_check(k-(k/2)*2 == (k&0x1));

		Protected::DynamicPeeling (F, ta, tb, m, n, k, m&0x1, n&0x1, k&0x1, alpha, A, lda, B, ldb, beta, C, ldc, H, Cmin, Cmax);
#else
		size_t ww = (size_t)H.recLevel ;
		size_t m2 = (m >> ww) << (ww-1) ;
		size_t n2 = (n >> ww) << (ww-1) ;
		size_t k2 = (k >> ww) << (ww-1) ;

		Protected::WinogradCalc (F, ta, tb, m2, n2, k2, alpha, A, lda, B, ldb, beta, C, ldc, H);

		size_t mr = m -2*m2;
		size_t nr = n -2*n2;
		size_t kr = k -2*k2;

		FFLASFFPACK_check(m == m2*2+mr);
		FFLASFFPACK_check(n == n2*2+nr);
		FFLASFFPACK_check(k == k2*2+kr);

		Protected::DynamicPeeling2 (F, ta, tb, m, n, k, mr, nr, kr, alpha, A, lda, B, ldb, beta, C, ldc, H, Cmin, Cmax);
#endif
		return C;
	} // fgemm

} // FFLAS

#endif // __FFLASFFPACK_fflas_fflas_fgemm_winograd_INL
