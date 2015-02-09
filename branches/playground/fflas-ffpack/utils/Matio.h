/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/* Copyright (C) LinBox,FFLAS-FFPACK
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
 */

#ifndef __FFLASFFPACK_matio_H
#define __FFLASFFPACK_matio_H

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
//#include "fflas-ffpack/fflas/fflas.h"
#include "fflas_memory.h"

// Reading and writing matrices over double

#if 0
// Reading a matrice from a (eventually zipped) file
double * read_dbl(char * mat_file,int* tni,int* tnj)
{
	char *UT, *File_Name;
	int is_gzipped = 0;
	size_t s = strlen(mat_file);
	double* X;
	if ((mat_file[--s] == 'z') &&
	    (mat_file[--s] == 'g') &&
	    (mat_file[--s] == '.')) {
		is_gzipped = 1;
		File_Name = "/tmp/bbXXXXXX_";
		mkstemp(File_Name);
		UT = FFLAS::fflas_new<char>(s+34+strlen(File_Name));
		sprintf(UT,"gunzip -c %s > %s", mat_file, File_Name);
		system(UT);
		sprintf(UT,"\\rm %s", File_Name);
	} else
		File_Name = mat_file;

	FILE* FileDes = fopen(File_Name, "r");
	if (FileDes != NULL) {
		char * tmp = FFLAS::fflas_new<char>(200);// usigned long tni, tnj;
		fscanf(FileDes,"%d %d %s\n",tni, tnj, &tmp) ;
		int n=*tni;
		int p=*tnj;
		X = FFLAS::fflas_new<double>(n*p);
		for (int i=0;i<n*p;++i)
			X[i] = (double) 0;
		long i,j; long val;
		fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
		while(i && j) {
			X[p*(i-1)+j-1] = (double) val;
			fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
		}
	}

	fclose(FileDes);
	if (is_gzipped) system(UT);
	return X;
}

// Displays a matrix
std::ostream& write_dbl(std::ostream& c,
			double* E,
			int n, int m, int id)
{

	for (int i = 0; i<n;++i){
		for (int j=0; j<m;++j)
			c << *(E+j+id*i) << " ";
		c << std::endl;
	}
	return c << std::endl;
}
#endif
// Reading and writing matrices over field

// Reading a matrice from a (eventually zipped) file
template<class Field>
typename Field::Element_ptr read_field(const Field& F,char * mat_file,int* tni,int* tnj)
{
	char *UT = NULL, *File_Name;
	int is_gzipped = 0;
	size_t s = strlen(mat_file);
	typename Field::Element_ptr X = NULL;
	if ((mat_file[--s] == 'z') &&
	    (mat_file[--s] == 'g') &&
	    (mat_file[--s] == '.')) {
		is_gzipped = 1;
		char tmp_nam[] = "/tmp/bbXXXXXX_";
		File_Name  = tmp_nam;
		if (mkstemp(File_Name))
			printf("Error opening file]\n");

		UT = FFLAS::fflas_new<char>(s+34+strlen(File_Name));
		sprintf(UT,"gunzip -c %s > %s", mat_file, File_Name);
		if (system(UT))
			printf("Error uncompressing file\n");
		sprintf(UT,"\\rm %s", File_Name);
	} else
		File_Name = mat_file;
	FILE* FileDes = fopen(File_Name, "r");
	if (FileDes != NULL) {
		char  tmp [200];// unsigned long tni, tnj;
		if (fscanf(FileDes,"%d %d %199s\n",tni, tnj, tmp)<0)
			printf("Error Reading first line of file \n");
		int n=*tni;
		int p=*tnj;
		X = FFLAS::fflas_new<typename Field::Element>(n*p);
		for (int i=0;i<n*p;++i)
			F.assign(X[i], F.zero);
		long i,j; long val;
		if(fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val)<0)
			printf("Read Error\n");
		while(i && j) {
			F.init(X[p*(i-1)+j-1],val);
			if(fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val)<0)
				printf("Read Error\n");
		}
		fclose(FileDes);
	}

	if (is_gzipped)
		if (system(UT))
			printf("Error uncompressing file\n");
	if (UT != NULL)
		delete[] UT;
	return X;
}

template<class Field>
void read_field4(const Field& F,char * mat_file,int* tni,int* tnj,
		 typename Field::Element_ptr& NW,typename Field::Element_ptr& NE,
		 typename Field::Element_ptr& SW,typename Field::Element_ptr& SE)
{
	char *UT = NULL, *File_Name;
	int is_gzipped = 0;
	size_t s = strlen(mat_file);
	typename Field::Element_ptr X;
	if ((mat_file[--s] == 'z') &&
	    (mat_file[--s] == 'g') &&
	    (mat_file[--s] == '.')) {
		is_gzipped = 1;
		// XXX on fait pas ça !
		char tmp_nam [] = "/tmp/bbXXXXXX_";
		File_Name = tmp_nam ;
		mkstemp(File_Name);
		UT = FFLAS::fflas_new<char>(s+34+strlen(File_Name));
		sprintf(UT,"gunzip -c %s > %s", mat_file, File_Name);
		system(UT);
		sprintf(UT,"\\rm %s", File_Name);
	}
	else {
		File_Name = mat_file;
	}
	FILE* FileDes = fopen(File_Name, "r");
	if (FileDes != NULL) {
		char * tmp = FFLAS::fflas_new<char>(200);// usigned long tni, tnj;
		fscanf(FileDes,"%d %d %199s\n",tni, tnj, tmp) ;
		delete[] tmp;
		int n=*tni;
		int p=*tnj;
		int no2= n>>1;
		int po2 = p>>1;
		NW = FFLAS::fflas_new<typename Field::Element>(no2*po2);
		NE = FFLAS::fflas_new<typename Field::Element>(no2*(p-po2));
		SW = FFLAS::fflas_new<typename Field::Element>((n-no2)*po2);
		SE = FFLAS::fflas_new<typename Field::Element>((n-no2)*(p-po2));

		for (int i=0;i<no2*po2;++i)
			F.assign(NW[i],F.zero);
		for (int i=0;i<no2*(p-po2);++i)
			F.assign(NE[i],F.zero);
		for (int i=0;i<(n-no2)*po2;++i)
			F.assign(SW[i],F.zero);
		for (int i=0;i<(n-no2)*(p-po2);++i)
			F.assign(SE[i],F.zero);
		long i,j; long val;
		fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
		while(i && j) {
			if (i<=no2){
				if (j<=po2){
					F.init(NW[po2*(i-1)+j-1],val);
					fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
				}
				else{
					F.init(NE[po2*(i-1)+j-1-po2],val);
					fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
				}
			}
			else{
				if (j<=po2){
					F.init(SW[(p-po2)*(i-1-no2)+j-1],val);
					fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
				}
				else{
					F.init(SE[(p-po2)*(i-1-no2)+j-1-po2],val);
					fscanf(FileDes,"%ld %ld %ld\n",&i, &j, &val) ;
				}
			}
		}
		//    *A1 = NW;
		//*A2 = NE;
		//*A3 = SW;
		//*A4 = SE;

		fclose(FileDes);
	}

	if (is_gzipped) system(UT);
	if (UT != NULL)
		delete[] UT ;
}

// Displays a matrix
template<class Field>
std::ostream& write_field(const Field& F,std::ostream& c,
			  typename Field::ConstElement_ptr E,
			  int n, int m, int id, bool mapleFormat = false)
{

	double tmp;
	if (mapleFormat) c << "Matrix(" << n <<',' << m << ", [" ;
	for (int i = 0; i<n;++i){
		if (mapleFormat) c << '[';
		for (int j=0; j<m;++j){
			F.convert(tmp,*(E+j+id*i));
			c << tmp;
			if (mapleFormat && j<m-1) c << ',';
			c << ' ';
		}
		if (mapleFormat) c << ']';
		if (mapleFormat && i<n-1) c << ',';
		if (!mapleFormat) c << std::endl;
	}
	if (mapleFormat) c << "])";
	return c ;
}

// Displays a triangular matrix
//! @todo let the user choose to convert to a non destructive format (not double but long or Integer...)
#if 0
template<class Field>
std::ostream& write_field(const Field& F,std::ostream& c,
			  const FFLAS::FFLAS_UPLO uplo, const FFLAS::FFLAS_DIAG unit,
			  const typename Field::Element* E,
			  int n, int m, int id, bool mapleFormat = false)
{

	double tmp;
	if (mapleFormat) c << "Matrix(" << n <<',' << m << ",[";
	for (int i = 0; i<n;++i){
		if (mapleFormat) c << '[';
		// under diag
		for (int j=0; j<i ;++j){
			if (uplo == FFLAS::FflasLower)
				F.convert(tmp,*(E+j+id*i));
			else tmp = 0 ;
			c << tmp;
			if (mapleFormat && j<m-1) c << ',';
			c << ' ';
		}
		// on diag
		if (unit == FFLAS::FflasNonUnit)
			F.convert(tmp,*(E+i+id*i));
		else
			tmp = 1.;
		c << tmp;
		if (mapleFormat && i<m-1) c << ',';
		c << ' ';
		// over diag
		for (int j=i+1; j<m;++j){
			if (uplo == FFLAS::FflasUpper)
				F.convert(tmp,*(E+j+id*i));
			else
				tmp = 0 ;
			c << tmp;
			if (mapleFormat && j<m-1) c << ',';
			c << ' ';
		}
		if (mapleFormat) c << ']';
		if (mapleFormat && i<n-1) c << ',';
		if (!mapleFormat) c << std::endl;
	}
	if (mapleFormat) c << "])";
	return c ;
}
#endif

#endif //__FFLASFFPACK_matio_H