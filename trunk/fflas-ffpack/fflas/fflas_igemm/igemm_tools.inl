/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2013,2014  Pascal Giorgi
 *
 * Written by Pascal Giorgi <pascal.giorgi@lirmm.fr>
 * the code is inspired and adapted from the Eigen library
 * modified by BB <brice.boyer@lip6.fr>
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

#ifndef __FFLASFFPACK_fflas_igemm_igemm_tools_INL
#define __FFLASFFPACK_fflas_igemm_igemm_tools_INL

#include "fflas-ffpack/utils/simd.h"

namespace FFLAS { namespace details {

	template<>
	inline void duplicate_vect<2>(int64_t* XX, const int64_t* X, size_t n){
		int64_t *p=XX;
		for(size_t i=0;i<n;i++){
			p[0]=X[i];
			p[1]=X[i];
			p+=2;
		}
	}

	template<>
	inline void duplicate_vect<4>(int64_t* XX, const int64_t* X, size_t n){
		int64_t *p=XX;
		for(size_t i=0;i<n;i++){
			p[0]=X[i];
			p[1]=X[i];
			p[2]=X[i];
			p[3]=X[i];
			p+=4;
		}
	}

	// store each rows x k submatrices of Rhs in row major mode
	// if k does not divide cols, the remaining column are not packed
	template<size_t k>
	void pack_rhs(int64_t* XX, const int64_t* X, size_t ldx, size_t rows, size_t cols){
		size_t cols_by_k=(cols/k)*k;
		size_t p=0;
		// pack by k columns
		for(size_t j=0;j<cols_by_k;j+=k){
			for(size_t i=0;i<rows;i++)
				for (size_t l=0;l<k;l++,p++) {
					// std::cout << p << ',' << (i+(j+l)*ldx) << std::endl;
					XX[p]=X[i+(j+l)*ldx];
				}
		}
		// the remaining columns are not packed
		for(size_t j=cols_by_k;j<cols;j++)
			for(size_t i=0;i<rows;i++,p++) {
				// std::cout << p << ',' << i+j*ldx << std::endl;
				// std::cout << X[i+j*ldx] << std::endl;
				XX[p]=X[i+j*ldx];
			}
	}


	// store each k x cols submatrices of Lhs in column major mode
	// if k does not divide rows, the remaining rows are not packed
	template<size_t k>
	void pack_lhs(int64_t* XX, const int64_t* X, size_t ldx, size_t rows, size_t cols){
		using simd = Simd<int64_t> ;
		// using half_t =  typename simd::half_t;
		size_t p=0;
		size_t rows_by_k=(rows/k)*k;
		// pack rows by group of k
		for(size_t i=0;i<rows_by_k;i+=k)
			for(size_t j=0;j<cols;j++)
				// for (size_t l=0;l<k;l++,p++) XX[p]=X[i+l+j*ldx];
				for (size_t l=0;l<k;l+= simd::vect_size, p+=simd::vect_size){
					// __m128i T0,T1;
					// half_t T0,T1;
					// T0 = simd::loadu_half(&X[i+l+  j*ldx]);
					// SSE_LOADU(T0,X[i+l+  j*ldx]);
					// T1 = simd::loadu_half(&X[i+l+2+j*ldx]);
					// SSE_LOADU(T1,X[i+l+2+j*ldx]);
					// simd::store_half(&XX[p],T0);p+=2;
					// SSE_STORE(XX[p],T0);p+=2;
					// simd::store_half(&XX[p],T1);p+=2;
					// SSE_STORE(XX[p],T1);p+=2;
					simd::store(&XX[p],simd::loadu(&X[i+l+j*ldx]));
				}
		// the remaining rows are packed by group of StepA (if possible)
		if (rows-rows_by_k>=StepA){
			for(size_t j=0;j<cols;j++)
				// for (size_t l=0;l<StepA;l++,p++) XX[p]=X[rows_by_k+l+j*ldx];
				for (size_t l=0;l<StepA;l+=simd::vect_size,p+=simd::vect_size){
					// half_t T0;
					// __m128i T0;
					// T0 = simd::loadu_half(&X[rows_by_k+l+j*ldx]);
					// SSE_LOADU(T0,X[rows_by_k+l+j*ldx]);
					// simd::store_half(&XX[p],T0);p+=2;
					// SSE_STORE(XX[p],T0);p+=2;
					simd::store(&XX[p],simd::loadu(&X[rows_by_k+l+j*ldx]));
				}
			rows_by_k+=StepA;
		}
		for(size_t i=rows_by_k;i<rows;i++)
			for(size_t j=0;j<cols;j++,p++){
				XX[p]=X[i+j*ldx];
			}

	}

	void BlockingFactor(size_t& m, size_t& n, size_t& k)
	{
		int l1, l2, l3, tlb;
		getCacheSize(l1,l2,l3);
		getTLBSize(tlb);
		/*
		   cout<<"Cache size: ";
		   cout<<"L1 ("<<l1<<") ";
		   cout<<"L2 ("<<l2<<") ";
		   cout<<"L3 ("<<l3<<") ";
		   cout<<"TLB ("<<tlb<<") ";
		   cout<<endl;
		   */
		l2=std::max(l2,l3);
		l2=std::min(l2,tlb);
		size_t kc,mc;
		// kc * 2*(_mr+_nr) must fit in L1 cache
		// kc * (n+mc) must fit in L2 cache and in TLB
		size_t kdiv= 2*(_nr+_mr)*sizeof(int64_t);
		kc = std::min(k, l1/kdiv);
		mc = std::min(m, l2/(sizeof(int64_t) * kc));
		k=kc;
		m=mc;
		//cout<<"mc="<<m<<endl;
		//cout<<"kc="<<k<<endl;
	}


} // details
} // FFLAS

#endif // __FFLASFFPACK_fflas_igemm_igemm_tools_INL

