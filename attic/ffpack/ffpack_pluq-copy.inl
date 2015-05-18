/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/* ffpack/ffpack_pluq.inl
 * Copyright (C) 2012 Clement Pernet
 *
 * Written by Clement Pernet <Clement.Pernet@imag.fr>
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

#ifndef __FFLASFFPACK_ffpack_pluq_INL
#define __FFLASFFPACK_ffpack_pluq_INL

namespace FFPACK {

	template<class Field>
	inline size_t
	PLUQ_basecase (const Field& Fi, const FFLAS_DIAG Diag,
		       const size_t M, const size_t N,
		       typename Field::Element * A, const size_t lda,
		       typename Field::Element * B, const size_t ldb,
		       size_t*P, size_t *Q){
		size_t row = 0;
		size_t col = 0;
		size_t rank = 0;
		for (size_t i=0; i<M; ++i) P[i] = i;
		for (size_t i=0; i<N; ++i) Q[i] = i;
		while ((col < N)||(row < M)){
			size_t piv2 = rank;
			size_t piv3 = rank;
			typename Field::Element * A1 = A + rank*lda;
			typename Field::Element * A2 = A + col;
			typename Field::Element * A3 = A + row*lda;
			    // search for pivot in A2
			if (row==M){
				piv3=col;
			}else
				while ((piv3 < col) && Fi.isZero (A3 [piv3])) piv3++;
			if (piv3 == col){
				if (col==N){
					row++;
					continue;
				}
#ifdef LEFTLOOKING
				    // Left looking style update
				ftrsv (Fi, FflasLower, FflasNoTrans,
				       (Diag==FflasUnit)?FflasNonUnit:FflasUnit,
				       rank, A, lda, A2, lda);
				fgemv (Fi, FflasNoTrans, M-rank, rank, Fi.mOne,
				       A1,lda, A2, lda,
				       Fi.one, A2+rank*lda, lda);
#endif
				while ((piv2 < row) && Fi.isZero (A2 [piv2*lda])) piv2++;
				if (col<N) col++;
				if (piv2==M)
					continue;
			} else
				piv2 = row;
			if (row<M)  row++;
			if (Fi.isZero (A [piv2*lda+piv3])){
				    // no pivot found
				    //cerr<<endl;
				continue;
			}
			    // At this point the pivot is located at x=piv2 y = piv3
			P [rank] = piv2;
			Q [rank] = piv3;
			A2 = A+piv3;
			A3 = A+piv2*lda;
			typename Field::Element invpiv;
			Fi.inv (invpiv, A3[piv3]);
			if (Diag==FflasUnit){
#ifdef LEFTLOOKING
				    // Normalizing the pivot row
				for (size_t i=piv3+1; i<N; ++i)
					Fi.mulin (A3[i], invpiv);
#endif
			}
			else
				    // Normalizing the pivot column
				for (size_t i=piv2+1; i<M; ++i)
					Fi.mulin (A2 [i*lda], invpiv);
			    // Update
#ifndef LEFTLOOKING
			for (size_t i=piv2+1; i<M; ++i)
			 	for (size_t j=piv3+1; j<N; ++j)
					Fi.maxpyin (A[i*lda+j], A2[i*lda], A3[j]);
#endif
			    //Swapping pivot column
			if (piv3 > rank)
				for (size_t i=0; i<M; ++i){
					typename Field::Element tmp;
					Fi.assign (tmp, A[i*lda + rank]);
					Fi.assign (A[i*lda + rank], A2[i*lda]);
					Fi.assign (A2[i*lda], tmp);
				}
				    // Updating cols

			    //Swapping pivot row
			if (piv2 > rank)
				for (size_t i=0; i<N; ++i){
					typename Field::Element tmp;
					Fi.assign (tmp, A1[i]);
					Fi.assign (A1[i], A3[i]);
					Fi.assign (A3[i], tmp);
				}
#ifdef LEFTLOOKING
			    // Need to update the cols already updated
			for (size_t i=piv2+1; i<M; ++i)
				for (size_t j=piv3+1; j<col; ++j)
					Fi.maxpyin (A[i*lda+j],
						    A[i*lda+rank], A[rank*lda+j]);
#endif
			rank++;
		}
		MatrixCopy (Fi, M, N, B, ldb, A, lda);
		return rank;
	}

	template<class Field>
	inline size_t
	PLUQ (const Field& Fi, const FFLAS_DIAG Diag,
	      const size_t M, const size_t N,
	      typename Field::Element * A, const size_t lda,
	      typename Field::Element* B, size_t ldb,
	      size_t*P, size_t *Q){
//		write_field(Fi,std::cerr<<"Entrée A="<<std::endl,A,M,N,lda);

		for (size_t i=0; i<M; ++i) P[i] = i;
		for (size_t i=0; i<N; ++i) Q[i] = i;
		if (std::min (M,N) == 0) return 0;
		if (std::max (M,N) == 1){
			Fi.assign (*B,*A);
			return (Fi.isZero(*A))? 0 : 1;
		}
		if (M == 1){
			size_t piv = 0;
			while ((piv < N) && Fi.isZero (A[piv])) piv++;
			if (piv ==N){
				for (size_t i=0; i<N; ++i)
					Fi.assign (B[i], Fi.zero);
				return 0;
			}
			Q[0] = piv;
			Fi.assign (*B, A[piv]);

			for (size_t i=1; i<=piv; ++i)
				Fi.assign (B[i], Fi.zero);
			if (Diag== FflasUnit){
				typename Field::Element invpivot;
				Fi.inv(invpivot, *B);
				for (size_t i=piv+1; i<N; ++i)
					Fi.mul (B[i],A[i], invpivot);
			}
			else
				for (size_t i=piv+1; i<N; ++i)
					Fi.assign (B[i], A[i]);
			return 1;
		}
		if (N == 1){
			size_t piv = 0;
			while ((piv < M) && Fi.isZero (A[piv*lda])) piv++;
			if (piv == M){
				for (size_t i=0; i<M; ++i)
					Fi.assign (B[i*ldb], Fi.zero);
				return 0;
			}
			P[0] = piv;
			Fi.assign (*B, *(A+piv*lda));

			for (size_t i=1; i<=piv; ++i)
				Fi.assign (B[i*ldb], Fi.zero);
			if (Diag== FflasNonUnit){
				typename Field::Element invpivot;
				Fi.inv(invpivot, *B);
				for (size_t i=piv+1; i<M; ++i)
					Fi.mul (B[i*ldb], A[i*lda], invpivot);
			}
			else
				for (size_t i=piv+1; i<M; ++i)
					Fi.assign (B[i*ldb], A[i*lda]);
			return 1;
		}
#ifdef BASECASE_K
		if (std::min(M,N) < BASECASE_K)
			return PLUQ_basecase (Fi, Diag, M, N, A, lda, B, ldb, P, Q);
#endif
		FFLAS_DIAG OppDiag = (Diag == FflasUnit)? FflasNonUnit : FflasUnit;
		size_t M2 = M >> 1;
		size_t N2 = N >> 1;
		size_t * P1 = new size_t [M2];
		size_t * Q1 = new size_t [N2];
		size_t R1,R2,R3,R4;

		    // A1 = P1 [ L1 ] [ U1 V1 ] Q1
		    //        [ M1 ]
		typename Field::Element * B1 = new typename Field::Element[M2*N2];
		size_t ldb1=N2;
		R1 = PLUQ (Fi, Diag, M2, N2, A, lda, B1,ldb1, P1, Q1);
		typename Field::Element * A2 = A + N2;
		typename Field::Element * A3 = A + M2*lda;
		typename Field::Element * A4 = A3 + N2;
		typename Field::Element * F = A2 + R1*lda;
		typename Field::Element * G = A3 + R1;
		    // [ B1 ] <- P1^T A2
		    // [ B2 ]
		applyP (Fi, FflasLeft, FflasNoTrans, N-N2, 0, M2, A2, lda, P1);
		    // [ C1 C2 ] <- A3 Q1^T
		applyP (Fi, FflasRight, FflasTrans, M-M2, 0, N2, A3, lda, Q1);
		    // D <- L1^-1 B1
		ftrsm (Fi, FflasLeft, FflasLower, FflasNoTrans, OppDiag, R1, N-N2, Fi.one, B1, ldb1, A2, lda);
		    // E <- C1 U1^-1
		ftrsm (Fi, FflasRight, FflasUpper, FflasNoTrans, Diag, M-M2, R1, Fi.one, B1, ldb1, A3, lda);
		    // F <- B2 - M1 D
		fgemm (Fi, FflasNoTrans, FflasNoTrans, M2-R1, N-N2, R1, Fi.mOne, B1 + R1*ldb1, ldb1, A2, lda, Fi.one, A2+R1*lda, lda);
		    // G <- C2 - E V1
		fgemm (Fi, FflasNoTrans, FflasNoTrans, M-M2, N2-R1, R1, Fi.mOne, A3, lda, B1+R1, ldb1, Fi.one, A3+R1, lda);
		    // H <- A4 - ED
		fgemm (Fi, FflasNoTrans, FflasNoTrans, M-M2, N-N2, R1, Fi.mOne, A3, lda, A2, lda, Fi.one, A4, lda);
                    // F = P2 [ L2 ] [ U2 V2 ] Q2
		    //        [ M2 ]
		size_t * P2 = new size_t [M2-R1];
		size_t * Q2 = new size_t [N-N2];
		typename Field::Element * B2 = new typename Field::Element[M2*(N-N2)];
		size_t ldb2 = N-N2;
		R2 = PLUQ (Fi, Diag, M2-R1, N-N2, F, lda, B2, ldb2,  P2, Q2);
		    // G = P3 [ L3 ] [ U3 V3 ] Q3
		    //        [ M3 ]
		size_t * P3 = new size_t [M-M2];
		size_t * Q3 = new size_t [N2-R1];
		typename Field::Element * B3 = new typename Field::Element[(M-M2)*N2];
		size_t ldb3 = N2;
		R3 = PLUQ (Fi, Diag, M-M2, N2-R1, G, lda, B3, ldb3, P3, Q3);
		    // [ H1 H2 ] <- P3^T H Q2^T
		    // [ H3 H4 ]
		applyP (Fi, FflasRight, FflasTrans, M-M2, 0, N-N2, A4, lda, Q2);
		applyP (Fi, FflasLeft, FflasNoTrans, N-N2, 0, M-M2, A4, lda, P3);
//		write_field(Fi,cerr<<"A4="<<endl,A4,M-M2,N-N2,lda);
		    // [ E1 ] <- P3^T E
		    // [ E2 ]
		applyP (Fi, FflasLeft, FflasNoTrans, R1, 0, M-M2, A3, lda, P3);
		    // [ M11 ] <- P2^T M1
		    // [ M12 ]
		applyP (Fi, FflasLeft, FflasNoTrans, R1, 0, M2-R1, B1+R1*ldb1, ldb1, P2);
		    // [ D1 D2 ] <- D Q2^T
		applyP (Fi, FflasRight, FflasTrans, R1, 0, N-N2, A2, lda, Q2);
		    // [ V11 V12 ] <- V1 Q3^T
		applyP (Fi, FflasRight, FflasTrans, R1, 0, N2-R1, B1+R1, ldb1, Q3);
		    // I <- H U2^-1
		    // K <- H3 U2^-1
		ftrsm (Fi, FflasRight, FflasUpper, FflasNoTrans, Diag, M-M2, R2, Fi.one, B2, ldb2, A4, lda);
		    // J <- L3^-1 I (in a temp)
		typename Field::Element * temp = new typename Field::Element [R3*R2];
		for (size_t i=0; i<R3; ++i)
			fcopy (Fi, R2, temp + i*R2, 1, A4 + i*lda, 1);
		ftrsm (Fi, FflasLeft, FflasLower, FflasNoTrans, OppDiag, R3, R2, Fi.one, B3, ldb3, temp, R2);
		    // N <- L3^-1 H2
		ftrsm (Fi, FflasLeft, FflasLower, FflasNoTrans, OppDiag, R3, N-N2-R2, Fi.one, B3, ldb3, A4+R2, lda);
		    // O <- N - J V2
		fgemm (Fi, FflasNoTrans, FflasNoTrans, R3, N-N2-R2, R2, Fi.mOne, temp, R2, B2+R2, ldb2, Fi.one, A4+R2, lda);
		delete[] temp;
		    // R <- H4 - K V2 - M3 O
		typename Field::Element * R = A4 + R2 + R3*lda;
		fgemm (Fi, FflasNoTrans, FflasNoTrans, M-M2-R3, N-N2-R2, R2, Fi.mOne, A4+R3*lda, lda, B2+R2, ldb2, Fi.one, R, lda);
		fgemm (Fi, FflasNoTrans, FflasNoTrans, M-M2-R3, N-N2-R2, R3, Fi.mOne, B3+R3*ldb3, ldb3, A4+R2, lda, Fi.one, R, lda);
		    // H4 = P4 [ L4 ] [ U4 V4 ] Q4
		    //         [ M4 ]
		size_t * P4 = new size_t [M-M2-R3];
		size_t * Q4 = new size_t [N-N2-R2];
		typename Field::Element * B4 = new typename Field::Element[(M-M2-R3)*(N-N2-R2)];
		size_t ldb4 = N-N2-R2;
		R4 = PLUQ (Fi, Diag, M-M2-R3, N-N2-R2, R, lda, B4, ldb4, P4, Q4);
		    // [ E21 M31 0 K1 ] <- P4^T [ E2 M3 0 K ]
		    // [ E22 M32 0 K2 ]
		applyP (Fi, FflasLeft, FflasNoTrans, N2+R2, 0, M-M2-R3, A3+R3*lda, lda, P4);
		applyP (Fi, FflasLeft, FflasNoTrans, R3, 0, M-M2-R3, B3+R3*ldb3, ldb3, P4);
		    // [ D21 D22 ]     [ D2 ]
		    // [ V21 V22 ]  <- [ V2 ] Q4^T
		    // [  0   0  ]     [  0 ]
		    // [ O1   O2 ]     [  O ]
		applyP (Fi, FflasRight, FflasTrans, M2+R3, 0, N-N2-R2, A2+R2, lda, Q4);
		applyP (Fi, FflasRight, FflasTrans, R2, 0, N-N2-R2, B2+R2, ldb2, Q4);

		    // P <- Diag (P1 [ I_R1    ] , P3 [ I_R3    ])
		    //               [      P2 ]      [      P4 ]
		size_t* MathP = new size_t[M];
		composePermutationsP (MathP, P1, P2, R1, M2);
		composePermutationsP (MathP+M2, P3, P4, R3, M-M2);
		delete[] P1;
		delete[] P2;
		delete[] P3;
		delete[] P4;
		for (size_t i=M2; i<M; ++i)
			MathP[i] += M2;
		if (R1+R2 < M2){
			    // P <- P S
			applyS (MathP, 1,1,M2, R1, R2, R3, R4);
		}
		MathPerm2LAPACKPerm (P, MathP, M);
		delete[] MathP;

		    // Q<- Diag ( [ I_R1    ] Q1,  [ I_R2    ] Q2 )
		    //            [      Q3 ]      [      P4 ]
		size_t * MathQ = new size_t [N];
		composePermutationsQ (MathQ, Q1, Q3, R1, N2);
		composePermutationsQ (MathQ+N2, Q2, Q4, R2, N-N2);
		delete[] Q1;
		delete[] Q2;
		delete[] Q3;
		delete[] Q4;
		for (size_t i=N2; i<N; ++i)
			MathQ[i] += N2;

		if (R1 < N2){
			    // Q <- T Q
			applyT (MathQ, 1,1,N2, R1, R2, R3, R4);
		}
		MathPerm2LAPACKPerm (Q, MathQ, N);
		delete[] MathQ;

		    // Composition of the result matrix B
		    // L1\U1
		    // M1
		MatrixCopy (Fi, R1+R2, R1, B, ldb, B1, ldb1);
		    // D1
		MatrixCopy (Fi, R1, R2, B+R1, ldb, A+N2, lda);
		    // L2/U2
		MatrixCopy (Fi, R2, R2, B+R1*(ldb+1), ldb, B2, ldb2);
		    // V11
		MatrixCopy (Fi, R1+R2, R3, B+R1+R2, ldb, B1+R1, ldb1);
		    // D21
		MatrixCopy (Fi, R1, R4, B+R1+R2+R3, ldb, A+N2+R2, lda);
		    // V21
		MatrixCopy (Fi, R2, R4, B+R1+R2+R3+R1*ldb, ldb, B2+R2, ldb2);
		    // V12
		MatrixCopy (Fi, R1+R2, N2-R1-R3, B+R1+R2+R3+R4, ldb, B1+R1+R3, ldb1);
		    // D22
		MatrixCopy (Fi, R1, N-N2-R2-R4, B+N2+R2+R4, ldb, A+N2+R2+R4, lda);
		    // V22
		MatrixCopy (Fi, R2, N-N2-R2-R4, B+N2+R2+R4+R1*ldb, ldb, B2+R2+R4, ldb2);
		    // E1\\ E21
		MatrixCopy (Fi, R3+R4, R1, B+(R1+R2)*ldb, ldb, A+M2*lda, lda);
		    // I\\ K1
		MatrixCopy (Fi, R3+R4, R2, B+(R1+R2)*ldb+R1, ldb, A+M2*lda+N2, lda);
		    // L3\U3\\ M31
		MatrixCopy (Fi, R3+R4, R3, B+(R1+R2)*(ldb+1), ldb, B3, ldb3);

		    // O1
		MatrixCopy (Fi, R3, R4, B+(R1+R2)*(ldb+1)+R3, ldb, A+M2*lda+N2+R2, lda);
		    // V3
		MatrixCopy (Fi, R3, N2-R1-R3, B+(R1+R2)*(ldb+1)+R3+R4, ldb, B3+R3, ldb3);
		    // O2
		MatrixCopy (Fi, R3, N-N2-R2-R4, B+(R1+R2)*ldb + N2+R2+R4, ldb, A+M2*lda+N2+R2+R4, lda);
		    //L4\U4
		MatrixCopy (Fi, R4, R4, B+(R1+R2+R3)*(ldb+1), ldb, B4, ldb4);
		    // 0
		typename Field::Element* Bptr= B + R4 + (R1+R2+R3)*(ldb+1);
		for (size_t i=0; i<R4; ++i, Bptr += ldb)
			for (size_t j=0; j < N2-R1-R3; ++j)
				Fi.assign (Bptr[j], Fi.zero);
		    //V4
		MatrixCopy (Fi, R4, N-N2-R2-R4, B+(R1+R2+R3)*ldb + R2+R4+N2, ldb, B4+R4, ldb4);
		    //M12
		MatrixCopy (Fi, M2-R1-R2, R1, B+(R1+R2+R3+R4)*ldb, ldb, B1+(R1+R2)*ldb1, ldb1);
		    //M2
		MatrixCopy (Fi, M2-R1-R2, R2, B+(R1+R2+R3+R4)*ldb+R1, ldb, B2+R2*ldb2, ldb2);
		    // 0 0 0 0
		Bptr= B + R1 + R2 + (R1+R2+R3+R4)*ldb;
		for (size_t i=0; i<M2-R1-R2; ++i, Bptr += ldb)
			for (size_t j=0; j < N-R1-R2; ++j)
				Fi.assign (Bptr[j], Fi.zero);
		    //E22
		MatrixCopy (Fi, M-M2-R3-R4, R1, B+(M2+R3+R4)*ldb, ldb, A+(M2+R3+R4)*lda, lda);
		    //K2
		MatrixCopy (Fi, M-M2-R3-R4, R2, B+(M2+R3+R4)*ldb+R1, ldb, A+(M2+R3+R4)*lda+N2, lda);
		    //M32
		MatrixCopy (Fi, M-M2-R3-R4, R3, B+(M2+R3+R4)*ldb+R1+R2, ldb,  B3+(R3+R4)*ldb3, ldb3);
		    //M4
		MatrixCopy (Fi, M-M2-R3-R4, R4, B+(M2+R3+R4)*ldb+R1+R2+R3, ldb, B4+R4*ldb4, ldb4);
		    // 0 0 (Not necessary?)
		Bptr = B+ (M2+R3+R4)*ldb + R1+R2+R3+R4;
		for (size_t i=0; i<M-M2-R3-R4; ++i, Bptr += ldb)
			for (size_t j=0; j < N-R1-R2-R3-R4; ++j)
				Fi.assign (Bptr[j], Fi.zero);
		delete[] B1;
		delete[] B2;
		delete[] B3;
		delete[] B4;
//		write_field(Fi,std::cerr<<"Sortie B="<<std::endl,B,M,N,ldb);
		return R1+R2+R3+R4;


	}

	template <class Field>
	inline void MatrixCopy (const Field& F, const size_t M, const size_t N,
				typename Field::Element* B, const size_t ldb,
				const typename Field::Element* A, const size_t lda){
		for (size_t i=0; i<M; ++i)
			for (size_t j=0; j<N; ++j)
				F.assign (B[i*ldb + j], A[i*lda + j]);
	}

	template <class Element>
	inline void applyS (Element* A, const size_t lda, const size_t width,
			    const size_t M2,
			    const size_t R1, const size_t R2,
			    const size_t R3, const size_t R4){
		Element * tmp = new Element [(M2-R1-R2)*width];
		// std::cerr<<"ici"<<std::endl;
		for (size_t i = 0, j = R1+R2; j < M2; ++i, ++j)
			for (size_t k = 0; k<width; ++k)
				tmp [i*width + k] = A [j*lda + k];
		for (size_t i = M2, j = R1+R2; i < M2+R3+R4; ++i, ++j)
			for (size_t k = 0; k<width; ++k)
				A [j*lda + k] = A [i*lda +k];
		for (size_t i = 0, j = R1+R2+R3+R4; i < M2-R1-R2; ++i, ++j)
			for (size_t k = 0; k<width; ++k)
				A [j*lda + k] = tmp [i*width + k];
		delete[] tmp;
	}


	template <class Element>
	inline void applyT (Element* A, const size_t lda, const size_t width,
			    const size_t N2,
			    const size_t R1, const size_t R2,
			    const size_t R3, const size_t R4){
		Element * tmp = new Element[(N2-R1)*width];
		for (size_t k = 0; k < width; ++k){
			for (size_t i = 0, j = R1; j < N2; ++i, ++j){
				tmp [i + k*(N2-R1)] = A [k*lda + j];
			}

			for (size_t i = N2, j = R1; i < N2+R2; ++i, ++j)
				A [k*lda + j] = A [k*lda + i];

			for (size_t i = 0, j = R1+R2; i < R3; ++i, ++j)
				A [k*lda + j] = tmp [k*(N2-R1) + i];

			for (size_t i = N2+R2, j = R1+R2+R3; i < N2+R2+R4; ++i,++j)
				A [k*lda + j] = A [k*lda + i];
			for (size_t i = R3, j = R1+R2+R3+R4; i < N2-R1; ++i,++j)
				A [k*lda + j] = tmp [k*(N2-R1) + i];
		}
		delete[] tmp;
	}


            /**
	     * Conversion of a permutation from LAPACK format to Math format
	     */
	inline void LAPACKPerm2MathPerm (size_t * MathP, const size_t * LapackP,
				  const size_t N){
		for (size_t i=0; i<N; i++)
			MathP[i] = i;
		for (size_t i=0; i<N; i++){
			if (LapackP[i] != i){
				size_t tmp = MathP[i];
				MathP[i] = MathP[LapackP[i]];
				MathP[LapackP[i]] = tmp;
			}
		}
	}
	    /**
	     * Conversion of a permutation from Maths format to LAPACK format
	     */
	inline void MathPerm2LAPACKPerm (size_t * LapackP, const size_t * MathP,
					 const size_t N){
		size_t * T = new size_t[N];
		size_t * Tinv = new size_t[N];
		for (size_t i=0; i<N; i++){
			T[i] =i;
			Tinv[i] = i;
		}
		for (size_t i=0; i<N; i++){
			size_t j = Tinv [MathP [i]];
			LapackP [i] = j;
			size_t tmp = T[j];
			T[j] = T[i];
			Tinv[T[i]] = j;
			T[i] = tmp;
			Tinv[tmp] = i;
		}
		delete[] T;
		delete[] Tinv;
	}
	    /**
	     * Computes P1 [ I_R     ] stored in MathPermutation format
	     *             [     P_2 ]
	     */
	inline void composePermutationsP (size_t * MathP,
				  const size_t * P1,
				  const size_t * P2,
				  const size_t R, const size_t N){
		for (size_t i=0; i<N; ++i)
			MathP[i] = i;
		LAPACKPerm2MathPerm (MathP, P1, N);

		for (size_t i=R; i<N; i++){
			if (P2[i-R] != i-R){
				size_t tmp = MathP[i];
				MathP[i] = MathP[P2[i-R]+R];
				MathP[P2[i-R]+R] = tmp;
			}
		}
	}
	inline void composePermutationsQ (size_t * MathP,
				  const size_t * Q1,
				  const size_t * Q2,
				  const size_t R, const size_t N){
		for (size_t i=0; i<N; ++i)
			MathP[i] = i;
		LAPACKPerm2MathPerm (MathP, Q1, N);

		for (size_t i=R; i<N; i++){
			if (Q2[i-R] != i-R){
				size_t tmp = MathP[i];
				MathP[i] = MathP[Q2[i-R]+R];
				MathP[Q2[i-R]+R] = tmp;
			}
		}
	}
	inline void
	RankProfilesFromPLUQ (size_t* RowRankProfile, size_t* ColumnRankProfile,
			      const size_t * P, const size_t * Q,
			      const size_t M, const size_t N, const size_t R){
		size_t * RRP=new size_t[M];
		size_t * CRP=new size_t[N];
		for (size_t i=0;i < M; ++i)
			RRP [i]=i;
		for (size_t i=0;i < N; ++i)
			CRP [i]=i;

		std::cerr<<"CRP = ";
		for (size_t i=0; i<N; ++i)
			std::cerr<<CRP[i]<<" ";
		std::cerr<<std::endl;
		for (size_t i=0; i<M; ++i)
			if (P[i] != i){
				size_t tmp = RRP [i];
				RRP [i] = RRP [P [i]];
				RRP [P [i]] = tmp;
			}
		for (size_t i=0; i<R; ++i)
			RowRankProfile[i] = RRP[i];
		for (int i=0; i<N; ++i){
			std::cerr<<"Q["<<i<<"] ="<<Q[i]<<std::endl;
			if (Q[i] != i){
				size_t tmp = CRP [i];
				CRP [i] = CRP [Q [i]];
				CRP [Q [i]] = tmp;
			}
		}
		for (size_t i=0; i<R; ++i)
			ColumnRankProfile [i] = CRP [i];

		std::cerr<<"CRP = ";
		for (size_t i=0; i<N; ++i)
			std::cerr<<CRP[i]<<" ";
		std::cerr<<std::endl;
		delete[] RRP;
		delete[] CRP;
	}

} // namespace FFPACK
#endif // __FFLASFFPACK_ffpack_pluq_INL