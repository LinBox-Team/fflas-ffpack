/* -*- mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
// vim:sts=4:sw=4:ts=4:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/* Copyright (C) FFLAS-FFPACK 2017
 * written by Clement Pernet (clement.pernet@univ-grenoble-alpes.fr)
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
 */

#ifndef __FFLAS_IO_H
#define __FFLAS_IO_H

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
//#include "fflas-ffpack/fflas/fflas.h"
#include "fflas_memory.h"

// Reading and writing matrices over field

// Reading a matrice from a (eventually zipped) file
#define __FFLAS_FIRST_LINE_LIMIT 160

namespace FFLAS{

    enum FFLAS_FORMAT {
        FflasAuto     = 0, // Automated detection of the format
        FflasDense    = 1, // Dense format (Array)
        FflasSMS      = 2, // Sparse Matrix format (Matrix Market)
        FflasBinary   = 3, // Binary format
        FflasMath     = 4, // Standard math. notation
        FflasMaple    = 5, // Maple input
        FflasSageMath = 6  // SageMath input
    };
    

    void preamble(std::ifstream&ifs, FFLAS_FORMAT& format){
		char st[8];
		ifs.read(st, 8);
		if (!strcmp(st,"FFBinFmt")){
			format = FflasBinary;

		}else{

		}
		ifs.seekg(0);
		return;
			// Detect Dense and SMS formats
    }
    template<class Field>
    typename Field::Element_ptr
    ReadMatrix (std::ifstream& ifs, Field& F, size_t& m, size_t& n,
                typename Field::Element_ptr& A, FFLAS_FORMAT format = FflasAuto){

		FFLAS_FORMAT form = format;

            // Preamble analysis. Update form to the discovered format.
        preamble(ifs, form);
        switch (form){
            case FflasDense:
                ifs >> m;
                ifs >> n;
                A = fflas_new(F, m,n);
                for (size_t i=0; i<m*n; i++){
                    F.read (ifs, A[i]);
                }
                break;

            case FflasSMS:
                ifs >> m;
                ifs >> n;
                size_t i,j,tmp;
                ifs >> tmp;
                A = fflas_new(F, m,n);
                do{
                    ifs >> i >> j;
                    F.read (ifs, A[(i-1)*n+j-1]);
                } while (i!=0 && j!=0 && !F.isZero(A[(i-1)*n+j-1]));
                break;

            case FflasBinary:
				char st[8];
				ifs.read(st, 8);
				if (strcmp(st,"FFBinFmt")){
					std::cerr<<"Not a FFLAS-FFPACK binary matrix format: "<<std::endl;
					return NULL;
				}
				size_t mm, nn;
				ifs.read(reinterpret_cast<char *>(&mm), sizeof(size_t));
				ifs.read(reinterpret_cast<char *>(&nn), sizeof(size_t));
				m=mm,n=nn;
				A = fflas_new(F, m,n);
				ifs.read (reinterpret_cast<char *>(A), sizeof(typename Field::Element)*m*n);
				ifs.close();
                break;

            default:
                std::cerr<<"Unable to detect the file format"<<std::endl;
        }
        return A;
    }

    template<class Field>
    inline typename Field::Element_ptr
    ReadMatrix (const std::string& matrix_file, Field & F, size_t & m, size_t& n,
                typename Field::Element_ptr& A, FFLAS_FORMAT format = FflasAuto){

        std::ios_base::openmode mode = std::ios::in;
        if (format == FflasBinary)
            mode |= std::ifstream::binary;

        std::ifstream ifs (matrix_file, mode);
		if (ifs.is_open()){
			ReadMatrix (ifs, F, m, n, A, format);
			ifs.close();
		} else
			std::cerr<<"Error: unable to open file "<<matrix_file<<std::endl;

        return A;
    }

    template<class Field>
    std::ostream& WriteMatrix (std::ostream& c, const Field& F, size_t m, size_t n,
                               typename Field::ConstElement_ptr A, size_t lda,
                               FFLAS_FORMAT format = FflasMath,
                               bool column_major=false) {
		switch (format){
			case FflasSageMath:
				c << "Matrix (";
                if (F.characteristic() == 0)
                    c << "ZZ, ";
                else
					c << "GF("<<F.cardinality()<<"), ";
				c << m <<", " << n << ", [" ;
				for (size_t i = 0; i<m; ++i){
					c << '[';
					for (size_t j=0; j<n;++j){
						if (column_major) F.write (c, A[j*lda+i]);
						else F.write (c, A[i*lda+j]);
						if (j < n-1) c << ", ";
					}
					c << ']';
					if (i < m-1) c << ", ";
				}
				c << "]);\n";
				break;
			case FflasMaple:
				c << "Matrix (";
				c << m <<", " << n << ", [" ;
				for (size_t i = 0; i<m; ++i){
					c << '[';
					for (size_t j=0; j<n;++j){
						if (column_major) F.write (c, A[j*lda+i]);
						else F.write (c, A[i*lda+j]);
						if (j < n-1) c << ", ";
					}
					c << ']';
					if (i < m-1) c << ", ";
				}
				c << "]);\n";
				break;
			case FflasDense:
				c<<m<<" "<<n<<std::endl;
				for (size_t i = 0; i<m; ++i){
					for (size_t j=0; j<n;++j){
						if (column_major) F.write (c, A[j*lda+i]);
						else F.write (c, A[i*lda+j]);
						c << ' ';
					}
				}
				c<<std::endl;
				break;
			case FflasSMS:
				c<<m<<' '<<n<<' '<<F.cardinality()<<std::endl;
				for (size_t i = 0; i<m; ++i){
					for (size_t j=0; j<n;++j){
						typename Field::ConstElement_ptr x;
						if (column_major) x= A + j*lda+i;
						else x = A + i*lda +j;
						if (!F.isZero(*x)){
							c << (i+1) << ' ' << (j+1) << ' ';
							F.write (c, *x);
							c << std::endl;
						}
					}
				}
				c << "0 0 0"<<std::endl;
				break;
			case FflasBinary:{
				char st[9]="FFBinFmt";
				c.write (st, 8);
				c.write ( reinterpret_cast<char*> (&m), sizeof (size_t));
				c.write ( reinterpret_cast<char*> (&n), sizeof (size_t));
				c.write ( reinterpret_cast<const char*> (A), sizeof(typename Field::Element)*m*n);
				break;
			}
			default: // format == FflasMath
				for (size_t i = 0; i<m; ++i){
					for (size_t j=0; j<n;++j){
						if (column_major) F.write (c, A[j*lda+i]);
						else F.write (c, A[i*lda+j]);
						if (j < n-1) c << ' ';
					}
					c<<std::endl;
				}
			}
		return c ;
    }
    
    template<class Field>
    void WriteMatrix (std::string& matrix_file, const Field& F, int m, int n,
                      typename Field::ConstElement_ptr A, size_t lda,
                      FFLAS_FORMAT format = FflasDense,
                      bool column_major=false) {

		std::ios_base::openmode mode = std::ios::out;
        if (format == FflasBinary)
			mode |= std::ios::binary;

        std::ofstream ofs (matrix_file, mode);
        if (ofs.is_open()){
			WriteMatrix (ofs, F, m, n, A, lda, format, column_major);
			ofs.close();
		} else
			std::cerr<<"Error: unable to open file "<<matrix_file<<std::endl;
    }
    
} //namespace FFLAS

#endif //__FFLAS_IO_H
