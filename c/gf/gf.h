// -----------------------------------------------------------------------------
// Core functions and algorithms to calculate in GF(2^n).
//
// Copyright (C) 2012 Till Schmalmack
// Uses several algorithms and ideas presented in this awesome book:
// Hans Kurzweil: Endliche Koerper (German)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#ifndef _GF_H
#define _GF_H

#include <ecc_cfg.h>	// needed for GF_N = 2^n (field size)

#ifndef GF_N
  #warning "Unspecified field size, using default: 16."
  #define GF_N 16
#endif

// Elements of our GF(2^n) are represented in two different formats, depending
// on the context:
// - exponent representation (gfExp):
//   exponent of a primitive element "z" plus 1; 0 represented as 0:
//    x = 0     ->   0
//    x = z^i   ->   i+1 (0 <= i < 2^n - 1)
//   (better suited for multiplication and division)
// - vector ("binary") representation (gfVec):
//    x = x{n-1} * v{n-1}  + ... + x0 * v0
//    ->  x{n-1} * 2^{n-1} + ... + x0 * 2^0)
//   with xi in {0, 1} and any valid base of our GF (v{n-1}, ..., v0), vi in GF
//   (better suited for adding/subtracting)
//   
// With m := GF_N - 1:
//	value				exp.	macros			
//						repr.									
//-----------------------------------------------------------
//	0					0		 GF_0						
//	1 = z^0				1		 GF_1		GF_Z(0)			
//	z^1					2					GF_Z(1)			
//	z^i					i + 1				GF_Z(i)			
//	z^(m-1)				m					GF_Z(m-1)		
//																
//	z^(-1) = z^(m-1)	m					GF__Z(-1)		
//	z^(-i) = z^(m-i)	m-(i-1)				GF__Z(-i)		
//	z^(-(m-1)) = z^1	2					GF__Z(-(m-1))


typedef int   gfExp;	// exponent representation
typedef gfExp gfVec;	// vector representation

// lookup tables to convert between both representations:
extern gfVec gfE2V[GF_N];	// z^i -> vector
extern gfExp gfV2E[GF_N];	// vector -> z^i
// extern gfExp zechLog[GF_N];	// was used by gfAdd() but turned out to be slower


// -----------------------------------------------------------------------------
// enable/disable basic printf debugging:
// -----------------------------------------------------------------------------
#ifdef DEBUG
  #include <stdio.h>
  #define PRINTPOL(name, p, n) { \
	printf(name "[%3d] = { ", n); \
	for (int i=n; i>=0; i--) { \
		printf("%3d ", p[i]); \
	} \
	printf("}\n"); \
 }
  #define dprintf printf
#else
  #define dprintf
  #define PRINTPOL
#endif


// =============================================================================
// basic operations (* / + -):
// =============================================================================

// Valid for vector- and exponent representations:
#define GF_0		0	// 0
#define GF_1		1	// z^0 = 1

// Map i -> exp. representation of z^i
// 0 <= i < GF_N-1
// -----------------------------------------------------------------------------
inline gfExp GF_Z(int i)
// -----------------------------------------------------------------------------
{
  #ifdef DEBUG
	// sanity checks
	if (i >= GF_N - 1)
		printf("ERROR: GF_Z: %d >= GF_N-1 !!\n", i);
  #endif
	return i + 1;	// map exponent i to exp. repr. of z^i
}

// Same that also works for negative exponents
// !! to be used only with constant i (otherwise too expensive) !!
// -----------------------------------------------------------------------------
inline gfExp GF__Z(int i)
// -----------------------------------------------------------------------------
{
  #ifdef DEBUG
	// sanity checks
	if (i < -GF_N + 1)
		printf("ERROR: GF__Z: %d < -GF_N+1 !!\n", i);
  #endif
	// a % b behaves different for neg. values  ->  shift to a>=0
	return (((i + GF_N - 1) % (GF_N - 1)) + 1);
}


// a * b; a!=0, b!=0
// -----------------------------------------------------------------------------
inline gfExp gfMul11(gfExp a, gfExp b)
// -----------------------------------------------------------------------------
{
  #ifdef DEBUG
	// sanity checks
	if (a == GF_0)
		printf("ERROR: gfMul11: a=0 !!\n");
	if (b == GF_0)
		printf("ERROR: gfMul11: b=0 !!\n");
  #endif
	gfExp r = a + b - 1;
	if (r >= GF_N)
		r -= (GF_N - 1);
	return r;
}


// a * b; b!=0
// -----------------------------------------------------------------------------
inline gfExp gfMul01(gfExp a, gfExp b)
// -----------------------------------------------------------------------------
{
	if (a == GF_0)
		return GF_0;
	return gfMul11(a, b);
}


// a * b
// -----------------------------------------------------------------------------
inline gfExp gfMul(gfExp a, gfExp b)
// -----------------------------------------------------------------------------
{
	if (b == GF_0)
		return GF_0;
	return gfMul01(a, b);
}


// a / b; a!=0
// -----------------------------------------------------------------------------
inline gfExp gfDiv1(gfExp a, gfExp b)
// -----------------------------------------------------------------------------
{
  #ifdef DEBUG
	// sanity checks
	if (a == GF_0)
		printf("ERROR: gfDiv1: a=0 !!\n");
	if (b == GF_0)
		printf("ERROR: gfDiv1: b=0 (division by zero) !!\n");
  #endif
	gfExp r = a - b + 1;
	if (r <= 0)
		r += (GF_N - 1);
	return r;
}


// a^(-1), a!=0
// may be faster than gfDiv1(GF_1, a), depending on compiler
// -----------------------------------------------------------------------------
inline gfExp gfInv1(gfExp a)
// -----------------------------------------------------------------------------
{
  #ifdef DEBUG
	// sanity checks
	if (a == GF_0)
		printf("ERROR: gfInv1: a=0 (division by zero) !!\n");
  #endif
	if (a == GF_1)
		return GF_1;
	return GF_N + 1 - a;
}


// a / b
// -----------------------------------------------------------------------------
inline gfExp gfDiv(gfExp a, gfExp b)
// -----------------------------------------------------------------------------
{
	if (a == GF_0)
		return GF_0;
	return gfDiv1(a, b);
}


// a + b
// -----------------------------------------------------------------------------
inline gfExp gfAdd(gfExp a, gfExp b)
// -----------------------------------------------------------------------------
{
	return gfV2E[gfE2V[a] ^ gfE2V[b]];
// Smarter but slower :( using the Zech-logarithm.  It requires only one table
// lookup (instead of 3) but apparently more expensive 0-checks.
// 	if (a == GF_0)
// 		return b;
// 	if (b == GF_0)
// 		return a;
// 	if (a == b)
// 		return GF_0;
// 	// otherwise, use the Zech-logarithm (from table):
// 	// z^i + z^j = z^i * (1 + z^(j-i))
// 	// <=> log(z^i + z^j) = i + log(1 + z^(j-i))
// 	//                          `-zechLog[j-i]-'
// 	gfExp b_a = gfDiv(b, a);	// can't be z^0 (a==b already catched)
// 	gfExp zl = zechLog[b_a];	// => can't be GF_0
// 	gfExp r = a + zl;
// 	if (r >= GF_N)
// 		r -= (GF_N - 1);
// 	return r;
}
#define gfSub gfAdd	// true for char(GF) == 2


// -----------------------------------------------------------------------------
// initialize LUTs gfExp <-> gfVec
// -----------------------------------------------------------------------------
int gfInit();


// =============================================================================
// polynomial arithmetics:
// =============================================================================


// return degree of polynomial A, with
//   deg(A) = 0 for A = a0 != 0
//   deg(A) = -0 for A = 0
// -----------------------------------------------------------------------------
inline int gfPolDeg(
	gfExp*	A,	// polynomial
	int		nA)	// max. deg(A)
// -----------------------------------------------------------------------------
{
	while ((nA >= 0) && (A[nA] == GF_0))
		nA--;
	return nA;
}


// return 1 if A=0
// -----------------------------------------------------------------------------
inline int gfPolIsZero(
	gfExp*	A,	// polynomial
	int		nA)	// max. deg(A)
// -----------------------------------------------------------------------------
{
	return (gfPolDeg(A, nA) == -1);
}


// convert polynomial from vector to exponent representation
// -----------------------------------------------------------------------------
inline void gfPolV2E(
	gfVec* Av,	// in:  polynomial in vector representation
	gfExp* A,	// out: polynomial in exponent representation; A=Av is supported
	int nA)		// max. deg(A)
// -----------------------------------------------------------------------------
{
	for (int ia=nA; ia>=0; ia--)
		*(A++) = gfV2E[*(Av++)];
}


// convert polynomial from exponent to vector representation
// -----------------------------------------------------------------------------
inline void gfPolE2V(
	gfExp* A,	// in:  polynomial in exponent representation
	gfVec* Av,	// out: polynomial in vector representation; Av=A is supported
	int nA)		// max. deg(A)
// -----------------------------------------------------------------------------
{
	for (int ia=nA; ia>=0; ia--)
		*(Av++) = gfE2V[*(A++)];
}


// compute S = A + B; return actual deg(S)
// -----------------------------------------------------------------------------
int gfPolAdd(
	gfExp*	A,	// 1st pol
	int		nA,	// max. deg(A) -- actual degree may be less!
	gfExp*	B,	// 2nd pol
	int		nB,	// max. deg(B)    /
	gfExp*	S);	// sum; deg(S) is returned


// polynomial multiplication C = A * B
// -----------------------------------------------------------------------------
int gfPolMul(
	gfExp*	A,	// factor	[1, 2, 3]
	int		nA,	// max deg(A)
	gfExp*	B,	// factor	[4, 5]
	int		nB,	// max deg(B)
	gfExp*	C);	// product; deg(C) is returned


// optimized version of polDiv() for the special case where caller
// knows that deg(B) = nB and B[nB] = 1 (normalized)
// -----------------------------------------------------------------------------
int gfPolDiv1(
	gfExp*	A,	// in: numerator
	int		nA,	// in: max. deg(A)
	gfExp*	B,	// in: denominator
	int		nB,	// in: actual deg(B)
	gfExp*	R);	// out: remainder, has to be allocated from -1 .. nB-1 !!


// evaluate polynomial at a given location
// -----------------------------------------------------------------------------
gfExp gfPolEval(
	gfExp*	A,	// polynomial
	int		nA,	// degree of A
	gfExp	x);	// location


// evaluate polynomial A(X) at nY+1 sequential locations X = x * z^i, i=0..nY
// -----------------------------------------------------------------------------
void gfPolEvalSeq(
	gfExp*	A,	// polynomial
	int		nA,	// deg(A) !! limited to [0 ... GF_N-1[  !!
	gfVec*	Yv,	// result array (vector repr.) with Yv[nY] = A(x) ... Yv[0] = A(x * z^nY)
	int		nY,	// number of locations - 1 (or deg(Y))
	gfExp	x);	// 1st location


// extended Euclidean algorithm
// compute P, Q so that
//   P * N' = Q * A' = lcm(N', A')
// and
//   gcd(P, Q) = 1
// -----------------------------------------------------------------------------
void gfPolEEA(
	gfExp*	N,	// in; only highest coeffs of N'
	int		nN,	// in: actual deg(N)
	gfExp*	A,	// in; only highest coeffs of A'
	int		nA,	// in: actual deg(A)
	gfExp*	P,	// out
	int		*nP,// out: actual deg(P)
	gfExp*	Q,	// out
	int		*nQ,// in: expected deg(Q) = deg(N') - deg(A')
	gfExp*	M);	// free memory of size: 8 * (nN+1) + 3
				// out: actual deg(Q)	// FIXME: check if needed/useful


// Evaluate derivation A'(X) at X=x
// In GF(2^N), A'(X) computes to:
//   A(X)  = sum(ai * X^i),     i=0,1,2,3,4,..., deg(A)
//   A'(X) = sum(ai * X^(i-1)), i=  1,  3,  ..., deg(A) (odd coeffs with even powers)
// -----------------------------------------------------------------------------
gfExp gfPolEvalDeriv(
	gfExp*	A,	// polynomial
	int		nA,	// max. deg(A)
	gfExp	x);	// location

#endif	// _GF_H
