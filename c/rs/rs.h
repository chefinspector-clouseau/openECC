// -----------------------------------------------------------------------------
// Reed-Solomon encoder and decoder functions.
//
// Copyright (C) 2012 Till Schmalmack
// It is an implementation of the algorithm presented in this awesome book:
// Hans Kurzweil: Endliche Koerper (German)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#ifndef _RS_H
#define _RS_H

#include <ecc_cfg.h>
#include <gf/gf.h>

// Compute generator polynomial and super polynomial
// (may instead be done at compile time):
// rsGen(X) = prod(X - z^i) for i = 1 ... n-k  (aka D(X))
// rsSup(X) = prod(X - z^i) for i = 0 ... m-1  (aka N(X))
//          = X^m - 1
// -----------------------------------------------------------------------------
void rsInit();


// Compute check symbols from information symbols.
// Compute R(X) = (X^(n-k) * A(X)) % D(X)
// Codeword is then C = (A, R)
// deg(A) <= k-1, deg(R) <= n-k-1
// !! Abuses some memory at negative indices of A and R, see below !!
// !! A and R must NOT overlap !!
// -----------------------------------------------------------------------------
void rsEncode(
	gfExp* A,	// !! A[k-1]   ... A[0]   A[-1] ... A[-(n-k)] !!
	gfExp* R);	// !! R[n-k-1] ... R[0]   R[-1]               !!
				// !! <-- user part -->   <---- abused  ---->
// -----------------------------------------------------------------------------


// Compute information word from code word, correcting up to n-k/2 errors.
// -----------------------------------------------------------------------------
void rsDecode(
	gfExp* C);	// inout: codeword  C[n-1] ... C[n-k] C[n-k-1] ... C[0]
				// = info word      A[k-1] ... A[0]
// -----------------------------------------------------------------------------
#endif	// _RS_H
