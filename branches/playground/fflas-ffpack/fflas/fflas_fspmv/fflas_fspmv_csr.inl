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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

/** @file fflas/fflas_fspmv_csr.inl
 * NO DOC
*/

#ifndef __FFLASFFPACK_fflas_fflas_spmv_csr_INL
#define __FFLASFFPACK_fflas_fflas_spmv_csr_INL

namespace FFLAS { /*  CSR */

	template<class Element>
	struct CSR {
		size_t m ;
		size_t n ;
		index_t  * st  ;
		index_t  * col ;
		Element * dat ;
		// int mc ;
		// int ml ;
	};

	template<class Element>
	struct CSR_sub : public CSR<Element> {
		// size_t i0 ;
		// size_t j0 ;
	};

	template<class Element>
	struct CSR_ZO {
		size_t m ;
		size_t n ;
		index_t  * st  ;
		index_t  * col ;
		// Element * dat ;
		Element cst ;
		// size_t i0 ;
		// size_t j0 ;
	};

	// y = A x + b y ; (generic)
	namespace details {
		template<class Field>
		void sp_fgemv(
			      const Field& F,
			      // const FFLAS_TRANSPOSE tA,
			      const size_t m,
			      const size_t n,
			      const index_t * st,
			      const index_t * col,
			      const typename Field::Element *  dat,
			      const typename Field::Element * x ,
			      const typename Field::Element & b,
			      typename Field::Element * y
			     )
		{
#if 0
			if (tA == FflasNoTrans)  {
#endif
				for (size_t i = 0 ; i < m ; ++i) {
					if (! F.isOne(b)) {
						if (F.isZero(b)) {
							F.assign(y[i],F.zero);
						}
						else if (F.isMone(b)) {
							F.negin(y[i]);
						}
						else {
							F.mulin(y[i],b);
						}
					}
					// XXX can be delayed
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						F.axpyin(y[i],dat[j],x[col[j]]);
				}
#if 0
			}
			else {
				if (F.isZero(b)) {
					for (size_t i = 0 ; i < m ; ++i) {
						F.assign(y[i],F.zero);
					}
				}
				else if (F.isMone(b)) {
					for (size_t i = 0 ; i < m ; ++i) {
						F.negin(y[i]);
					}
				}
				for (size_t i = 0 ; i < m ; ++i) {
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						F.axpyin(y[col[j]],dat[j],x[j]);
				}
			}
#endif
		}

		// Double
		template<>
		void sp_fgemv(
			      const DoubleDomain & F,
			      // const FFLAS_TRANSPOSE tA,
			      const size_t m,
			      const size_t n,
			      const index_t * st,
			      const index_t * col,
			      const double *  dat,
			      const double * x ,
			      const double & b,
			      double * y
			     )
		{
#ifdef __FFLASFFPACK_HAVE_MKL
			// char * transa = (ta==FflasNoTrans)?'n':'t';
			char   transa = 'N';
			index_t m_ = (index_t) m ;
			double * yd ;
			if ( b == 0) {
				yd = y;
			}
			else {
				yd = FFLAS::fflas_new<double >(m);
				fscalin(F,m,b,y,1);
			}
			// mkl_dcsrgemv (bug, not zero based)
			mkl_cspblas_dcsrgemv
			(&transa, &m_, const_cast<double*>(dat), const_cast<index_t*>(st) , const_cast<index_t*>(col), const_cast<double*>(x), yd);
			if ( b != 0) {
				faddin(F,m,yd,1,y,1);
				delete[] yd ;
			}
#else
			for (size_t i = 0 ; i < m ; ++i) {
				if ( b != 1) {
					if ( b == 0.) {
						y[i] = 0;
					}
					else if ( b == -1 ) {
						y[i]= -y[i];
					}
					else {
						y[i] = y[i] * b;
					}
				}
				for (index_t j = st[i] ; j < st[i+1] ; ++j)
					y[i] += dat[j] * x[col[j]];
			}
#endif // __FFLASFFPACK_HAVE_MKL
		}


		// Float
		template<>
		void sp_fgemv(
			      const FloatDomain & F,
			      // const FFLAS_TRANSPOSE tA,
			      const size_t m,
			      const size_t n,
			      const index_t * st,
			      const index_t * col,
			      const float *  dat,
			      const float * x ,
			      const float & b,
			      float * y
			     )
		{
#ifdef __FFLASFFPACK_HAVE_MKL
			// char * transa = (ta==FflasNoTrans)?'n':'t';
			char   transa = 'n';
			index_t m_ = (index_t) m ;
			float * yd ;
			if ( b == 0) {
				yd = y;
			}
			else {
				yd = FFLAS::fflas_new<float >(m);
				fscalin(F,m,b,y,1);
			}
			// mkl_scsrgemv
			mkl_cspblas_scsrgemv
			(&transa, &m_, const_cast<float*>(dat), const_cast<index_t*>(st), const_cast<index_t*>(col), const_cast<float*>(x), yd);
			if ( b != 0) {
				faddin(F,m,yd,1,y,1);
				delete[] yd ;
			}
#else
			for (size_t i = 0 ; i < m ; ++i) {
				if ( b != 1) {
					if ( b == 0.) {
						y[i] = 0;
					}
					else if ( b == -1 ) {
						y[i]= -y[i];
					}
					else {
						y[i] = y[i] * b;
					}
				}
				for (index_t j = st[i] ; j < st[i+1] ; ++j)
					y[i] += dat[j] * x[col[j]];
			}
#endif // __FFLASFFPACK_HAVE_MKL
		}

#ifdef __FFLASFFPACK_HAVE_CUDA
		// Double
		template<>
		void sp_fgemv(
			      const DoubleDomain & F,
			      // const FFLAS_TRANSPOSE tA,
			      const size_t m,
			      const size_t n,
			      const index_t * st_d,
			      const index_t * col_d,
			      const double *  dat_d,
			      const double * x_d ,
			      const double & b,
			      double * y_d,
			      const cusparseMatDescr_t & descrA,
			      const cusparseHandle_t & handle
			     )
		{
			// char * transa = (ta==FflasNoTrans)?'n':'t';
			size_t nnz = st[m]-st[m-1];
			double one = 1.f;
			status= cusparseDcsrmv(handle,CUSPARSE_OPERATION_NON_TRANSPOSE, m, n, nnz,
					       &one, descrA, dat_d, st_d, col_d,
					       x_d, &b, y_d);
		}

		// Float
		template<>
		void sp_fgemv(
			      const FloatDomain & F,
			      // const FFLAS_TRANSPOSE tA,
			      const size_t m,
			      const size_t n,
			      const index_t * st_d,
			      const index_t * col_d,
			      const float *  dat_d,
			      const float * x_d ,
			      const float & b,
			      float * y_d,
			      const cusparseMatDescr_t & descrA,
			      const cusparseHandle_t & handle
			     )
		{
			// char * transa = (ta==FflasNoTrans)?'n':'t';
			size_t nnz = st[m]-st[m-1];
			float one = 1.f;
			status= cusparseScsrmv(handle,CUSPARSE_OPERATION_NON_TRANSPOSE, m, n, nnz,
					       &one, descrA, dat_d, st_d, col_d,
					       x_d, &b, y_d);

		}

		// need cuda finit code (need nvcc)
#endif // __FFLASFFPACK_HAVE_CUDA


		// delayed by kmax
		template<class Field>
		void sp_fgemv(
			      const Field& F,
			      // const FFLAS_TRANSPOSE tA,
			      const size_t m,
			      const size_t n,
			      const index_t * st,
			      const index_t * col,
			      const typename Field::Element *  dat,
			      const typename Field::Element * x ,
			      // const typename Field::Element & b,
			      typename Field::Element * y,
			      const index_t & kmax
			     )
		{
			for (size_t i = 0 ; i < m ; ++i) {
				// y[i] = 0;
				index_t j = st[i];
				index_t j_loc = j;
				index_t j_end = st[i+1];
				index_t block = (j_end - j_loc)/kmax ;
				for (size_t l = 0 ; l < (size_t) block ; ++l) {
					j_loc += kmax ;
					for ( ; j < j_loc ; ++j) {
						y[i] += dat[j] * x[col[j]];
					}
					F.init(y[i],y[i]);
				}
				for ( ; j < j_end ; ++j) {
					y[i] += dat[j] * x[col[j]];
				}
				F.init(y[i],y[i]);
			}
		}

		// generic
		template<class Field, bool add>
		void sp_fgemv_zo(
				 const Field & F,
				 // const FFLAS_TRANSPOSE tA,
				 const size_t m,
				 const size_t n,
				 const index_t * st,
				 const index_t * col,
				 const typename Field::Element * x ,
				 // const typename Field::Element & b,
				 typename Field::Element * y
				)
		{
			for (size_t i = 0 ; i < m ; ++i) {
				// if ( b != 1) {
				// if ( b == 0.) {
				// F.assign(y[i], F.zero);
				// }
				// else if ( b == -1 ) {
				// F.negin(y[i]);
				// }
				// else {
				// F.mulin(y[i],b);
				// }
				// }
				if (add == true) {
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						F.addin(y[i], x[col[j]]);
				}
				else{
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						F.subin(y[i], x[col[j]]);
				}
				// F.init(y[i],y[i]);
			}
		}

		// Double
		template<bool add>
		void sp_fgemv_zo(
				 const DoubleDomain & ,
				 // const FFLAS_TRANSPOSE tA,
				 const size_t m,
				 const size_t n,
				 const index_t * st,
				 const index_t * col,
				 const double * x ,
				 // const double & b,
				 double * y
				)
		{
			for (size_t i = 0 ; i < m ; ++i) {
				// if ( b != 1) {
				// if ( b == 0.) {
				// y[i] = 0;
				// }
				// else if ( b == -1 ) {
				// y[i]= -y[i];
				// }
				// else {
				// y[i] = y[i] * b;
				// }
				// }
				if (add == true) {
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						y[i] +=  x[col[j]];
				}
				else
				{
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						y[i] -=  x[col[j]];
				}
			}
		}

		// Float
		template<bool add>
		void sp_fgemv_zo(
				 const FloatDomain & ,
				 // const FFLAS_TRANSPOSE tA,
				 const size_t m,
				 const size_t n,
				 const index_t * st,
				 const index_t * col,
				 const float * x ,
				 const float & b,
				 float * y
				)
		{
			for (size_t i = 0 ; i < m ; ++i) {
				// if ( b != 1) {
				// if ( b == 0.) {
				// y[i] = 0;
				// }
				// else if ( b == -1 ) {
				// y[i]= -y[i];
				// }
				// else {
				// y[i] = y[i] * b;
				// }
				// }
				if (add == true) {
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						y[i] +=  x[col[j]];
				}
				else
				{
					for (index_t j = st[i] ; j < st[i+1] ; ++j)
						y[i] -=  x[col[j]];
				}
			}
		}



	} // details

	// y = A x + b y ; (generic)
	// it is supposed that no reduction is needed.
	template<class Field>
	void sp_fgemv(
		      const Field& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<typename Field::Element> & A,
		      const VECT<typename Field::Element> & x,
		      const typename Field::Element & b,
		      VECT<typename Field::Element> & y
		     )
	{
		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,b,y.dat);
	}

	template<>
	void sp_fgemv(
		      const DoubleDomain & F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,b,y.dat);
	}

	template<>
	void sp_fgemv(
		      const FloatDomain & F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,b,y.dat);
	}

	template<>
	void sp_fgemv(
		      const FFPACK::Modular<double>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		// std::cout << "there" << std::endl;
		sp_fgemv(DoubleDomain(),A,x,b,y);
		finit(F,A.m,y.dat,1);
	}

	template<>
	void sp_fgemv(
		      const FFPACK::ModularBalanced<double>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		sp_fgemv(DoubleDomain(),A,x,b,y);
		finit(F,A.m,y.dat,1);

	}

	template<>
	void sp_fgemv(
		      const FFPACK::Modular<float>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		sp_fgemv(FloatDomain(),A,x,b,y);
		finit(F,A.m,y.dat,1);


	}

	template<>
	void sp_fgemv(
		      const FFPACK::ModularBalanced<float>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR_sub<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		sp_fgemv(FloatDomain(),A,x,b,y);
		finit(F,A.m,y.dat,1);

	}


	// y = A x + b y ; (generic)
	// reductions are delayed.
	template<class Field>
	void sp_fgemv(
		      const Field& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<typename Field::Element> & A,
		      const VECT<typename Field::Element> & x,
		      const typename Field::Element & b,
		      VECT<typename Field::Element> & y
		     )
	{
		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,b,y.dat);
	}

	template<>
	void sp_fgemv(
		      const DoubleDomain & F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,b,y.dat);
	}

	template<>
	void sp_fgemv(
		      const FloatDomain & F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,b,y.dat);
	}



	template<>
	void sp_fgemv(
		      const FFPACK::Modular<double>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		// std::cout << "here" << std::endl;
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one,FflasDouble) ;

		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,y.dat,(index_t) kmax);
	}

	template<>
	void sp_fgemv(
		      const FFPACK::ModularBalanced<double>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one,FflasDouble) ;

		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,y.dat,(index_t) kmax);
	}

	template<>
	void sp_fgemv(
		      const FFPACK::Modular<float>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one,FflasFloat) ;

		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,y.dat,(index_t)kmax);
	}

	template<>
	void sp_fgemv(
		      const FFPACK::ModularBalanced<float>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const CSR<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one,FflasFloat) ;

		details::sp_fgemv(F,A.m,A.n,A.st,A.col,A.dat,x.dat,y.dat,(index_t) kmax);
	}


	// this is the cst data special case.
	// Viewed as a submatrix.
	// it is assumed that no reduction is needed while adding.
	template<class Field>
	void sp_fgemv(
		      const Field & F,
		      // const FFLAS_TRANSPOSE tA,
		      CSR_ZO<typename Field::Element> & A,
		      const VECT<typename Field::Element > & x,
		      const typename Field::Element & b,
		      VECT<typename Field::Element > & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);

		FFLASFFPACK_check(!F.isZero(A.cst));

		if (A.cst == F.one) {
			details::sp_fgemv_zo<Field,true>(F,A.m,A.n,A.st,A.col,x.dat,y.dat);
		}
		else if (A.cst == F.mOne) {
			details::sp_fgemv_zo<Field,false>(F,A.m,A.n,A.st,A.col,x.dat,y.dat);
		}
		else {
			typename Field::Element * xd = FFLAS::fflas_new<typename Field::Element >(A.n) ;
			fscal(F,A.n,A.cst,x.dat,1,xd,1);
			details::sp_fgemv_zo<Field,true>(F,A.m,A.n,A.st,A.col,xd,y.dat);
		}

		finit(F,A.m,y.dat,1);
	}

} // FFLAS

#endif // __FFLASFFPACK_fflas_fflas_spmv_csr_INL
