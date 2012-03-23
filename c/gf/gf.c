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

#include "gf.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

gfVec gfE2V[GF_N];
gfExp gfV2E[GF_N];

// -----------------------------------------------------------------------------
// initialize LUTs gfExp <-> gfVec
// -----------------------------------------------------------------------------
int gfInit()
{
	// without X^n, X^(n-1) at bit 0
	#if (GF_N == 8)
		#define GFPOL 0x6	//                 110 (1): (x^3) + x + 1
	#elif (GF_N == 16)
		#define GFPOL 0xc	//                1100 (1): (x^4) + x + 1
	#elif (GF_N == 32)
		#define GFPOL 0x14	//              1 0100 (1): (x^5) + x^2 + 1
	#elif (GF_N == 64)
		#define GFPOL 0x30	//             11 0000 (1): (x^6) + x + 1
	#elif (GF_N == 128)
		#define GFPOL 0x44	//            100 0100 (1): (x^7) + x^4 + 1
	#elif (GF_N == 256)
		#define GFPOL 0xb8	//           1011 1000 (1): (x^8) + x^4 + x^3 + x^2 + 1
	#elif (GF_N == 512)
		#define GFPOL 0x110	//         1 0001 0000 (1): (x^9) + x^4 + 1
	#elif (GF_N == 1024)
		#define GFPOL 0x240	//        10 0100 0000 (1): (x^10) + x^3 + 1
	#elif (GF_N == 2048)
		#define GFPOL 0x500	//       101 0000 0000 (1): (x^11) + x^2 + 1
	#elif (GF_N == 4096)
		#define GFPOL 0xca0	//      1100 1010 0000 (1): (x^12) + x^6 + x^4 + x + 1
	#elif (GF_N == 8192)
		#define GFPOL 0x1b00//    1 1011 0000 0000 (1): (x^13) + x^4 + x^3 + x + 1
	#elif (GF_N == 16384)
		#define GFPOL 0x3500//   11 0101 0000 0000 (1): (x^14) + x^5 + x^3 + x + 1
	#elif (GF_N == 32768)
		#define GFPOL 0x6000//  110 0000 0000 0000 (1): (x^15) + x + 1
	#elif (GF_N == 65536)
		#define GFPOL 0xb400// 1011 0100 0000 0000 (1): (x^16) + x^5 + x^3 + x^2 + 1
	#else
	 #error "Field size not supported."
	#endif

	gfE2V[0] = 0;
	gfV2E[0] = 0;
	gfVec v = GF_N >> 1;
	for (gfExp e=0; e<GF_N-1; e++) {	// z^e, represented as e+1
		int c = v & 1;
		v >>= 1;
		if (c)
			v ^= GFPOL;
		gfE2V[e+1] = v;
		gfV2E[v] = e+1;
	}
}


// -----------------------------------------------------------------------------
// compute S = A + B; return actual deg(S)
// -----------------------------------------------------------------------------
int gfPolAdd(
	gfExp*	A,	// 1st pol
	int		nA,	// max. deg(A) -- actual degree may be less!
	gfExp*	B,	// 2nd pol
	int		nB,	// max. deg(B)    /
	gfExp*	S)	// sum; deg(S) is returned
// -----------------------------------------------------------------------------
{
	dprintf("---------- polAdd\n");
	PRINTPOL("add: A", A, nA);
	PRINTPOL("add: B", B, nB);

	// first sort A, B acc. to their length:
	gfExp* P1;	int nP1;			// shorter
	gfExp* P2;	int nP2;			// longer
	if (nA > nB) {
		P2 = A; nP2 = nA; P1 = B; nP1 = nB;
	} else {
		P2 = B; nP2 = nB; P1 = A; nP1 = nA;
	}

	int nS = -1;
	// common part
	for (int i=0; i<=nP1; i++) {
		S[i] = gfAdd(P1[i], P2[i]);
		if (S[i] != GF_0)
			nS = i;
	}
	// part that is only in P2:
	for (int i=nP1+1; i<=nP2; i++) {
		S[i] = P2[i];
		if (S[i] != GF_0)
			nS = i;
	}
	PRINTPOL("add: S", S, nS);
	return nS;
}
#define polSub polAdd	// true for char(GF) == 2


// -----------------------------------------------------------------------------
// polynomial multiplication C = A * B
// returns actual deg(C).
// C must not point to the same location as A or B, i.e.
//   gfPolMul(A, B, A)	// A *= B
// does NOT work!
// (FIXME: check feasibility, benefit)
//
//	                  a0*b2 a0*b1 a0*b0
//              a1*b2 a1*b1 a1*b0
//        a2*b2 a2*b1 a2*b0
//	a3*b2 a3*b1 a3*b0
//  -----------------------------------
//   c5    c4    c3    c2    c1    c0
//
// -----------------------------------------------------------------------------
int gfPolMul(
	gfExp*	A,	// in: factor	[1, 2, 3]
	int		nA,	// in: actual deg(A)
	gfExp*	B,	// in: factor	[4, 5]
	int		nB,	// in: actual deg(B)
	gfExp*	C)	// out: product; deg(C) = nA + nB is returned
// -----------------------------------------------------------------------------
{
	dprintf("---------- polMul\n");
	PRINTPOL("mul: A", A, nA);
	PRINTPOL("mul: B", B, nB);
	// OPT: omit this if caller knows actual degrees of A, B
	int nC = nA + nB;
  #ifdef DEBUG
	if (nA != gfPolDeg(A, nA))
		dprintf("ERROR: gfPolMul: wrong degree (A): %d !!\n", nA);
	if (nB != gfPolDeg(B, nB))
		dprintf("ERROR: gfPolMul: wrong degree (B): %d !!\n", nB);
  #endif
	for (int i=nC; i>=0; i--)
		C[i] = GF_0;
	for (int ia=nA; ia>=0; ia--) {
		gfExp a = A[ia];
		if (a == GF_0)
			continue;
		for (int ib=nB; ib>=0; ib--) {
			gfExp x = gfMul01(B[ib], a);
			C[ia+ib] = gfAdd(C[ia+ib], x);
		}
	}
done:
	PRINTPOL("mul: C", C, nC);
	return nC;
//  OPT: alternative impl. may be faster by saving some e2v conversions)
// 	!! NOT READY YET !!
// 	for (int i=0; i<=nA+nB; i++)
// 	{
// 		C[i] = GF_0;
// 		int jStart = MAX(0, i-nA);
// 		int jEnd   = MIN(i, nB);
// 		for (int j=jStart; j<=jEnd; j++)
// 		{
// 			gfExp x = gfMul(A[i-j], B[j]);
// 			C[i] = gfAdd(C[i], x);
// 		}
// 	}
}


// r_ is R[-1]
//
//   r2 r1 r0 r_
//   a5 a4 a3 a2 a1 a0 : 1 b2 b1 b0 = q2 q1 q0
// -    x  x  x  |  |
//   ----------- v  |
//      r2 r1 r0 r_ |
//    -    x  x  x  |
//      ----------- v
//         r2 r1 r0 r_
//       -    x  x  x
//         -----------
//            r2 r1 r0

// -----------------------------------------------------------------------------
// Optimized version of polDiv() with these assumptions/restrictions:
// - nB is actual deg(B)
// - B[nB] = 1 (normalized) -> no need to divide
// - B[i] != 0 for i=0..nB (!!)
// - Q is not returned
// - R[-1] is abused
// -----------------------------------------------------------------------------
int gfPolDiv1(
	gfExp*	A,	// in: numerator
	int		nA,	// in: max. deg(A)
	gfExp*	B,	// in: denominator
	int		nB,	// in: actual deg(B)
	gfExp*	R)	// out: remainder, has to be allocated from -1 .. nB-1 !!
// -----------------------------------------------------------------------------
{
	dprintf("---------- polDiv1\n");
	PRINTPOL("div: A", A, nA);
	PRINTPOL("div: B", B, nB);

	// remainder is more efficient in vector representation:
	gfVec* Rv = R;
	// use upper numerator coeffs as initial remainder:
	gfPolE2V(A + nA - nB + 1, Rv, nB-1);	// copy & convert

	for (int iq=nA-nB; iq>=0; iq--) {
		Rv[-1] = gfE2V[A[iq]];	// fetch next a
		gfVec qv = Rv[nB-1];
		if (qv == GF_0) {
			// catch 0-case here to use faster 0-free gfMul11() in the loop below
			for (int ir=nB-1; ir>=0; ir--) {
				Rv[ir] = Rv[ir-1];
			}
			continue;
		}
		gfExp q = gfV2E[qv];
		for (int ir=nB-1; ir>=0; ir--) {
			Rv[ir] = Rv[ir-1] ^ gfE2V[gfMul11(q, B[ir])];
		}
	}

	// convert result back to exponent representation:
	gfPolV2E(Rv, R, nB-1);

	PRINTPOL("div: R", R, nB-1);
	return nB;
}


// -----------------------------------------------------------------------------
// Evaluate polynomial A(X) at X=x
// Horner scheme:
// an * X^n + a_{n-1} * X^{n-1} + ... + a2 * X^2 + a1 * X + a0 =
// ((..((a_n)X + a_{n-1})X + ...)X + a1)X + a0
// -----------------------------------------------------------------------------
gfExp gfPolEval(
	gfExp*	A,	// polynomial
	int		nA,	// degree of A
	gfExp	x)	// location
// -----------------------------------------------------------------------------
{
	// using the Horner scheme:
	gfExp r = A[nA];
	for (int i=nA-1; i>=0; i--) {
		r = gfMul(r, x);
		gfExp a = A[i];
		r = gfAdd(r, a);
	}
	PRINTPOL("evl: A", A, nA);
	dprintf("A(%d)=%d\n", x, r);
	return r;
}


// -----------------------------------------------------------------------------
// Evaluate polynomial A(X) at nY+1 "sequential" locations X = x * z^i, i=0..nY
// Useful for DFT and finding roots.
// It is based on the Chien-search idea:  
//
//    l                          ia ->
//    |
//    v                  A[0]   A[1]                      A[2]                            A[nA]
//
// Y[nY]   = A(x)      = A[0] + A[1] * x                + A[2] * x^2                +...+ A[nA] * x^nA
// Y[nY-1] = A(x*z^1)  = A[0] + A[1] * x * z^1          + A[2] * x^2 * z^2          +...+ A[nA] * x^nA * z^nA
// Y[nY-2] = A(x*z^2)  = A[0] + A[1] * x * z^1 * z^1    + A[2] * x^2 * z^2 * z^2    +...+ A[nA] * x^nA * z^nA * z^nA
//                                         |        \                  |        \                        |          \
// Y[0]    = A(x*z^nY) = A[0] + A[1] * x * z^1 *..* z^1 + A[2] * x^2 * z^2 *..* z^2 +...+ A[nA] * x^nA * z^nA *..* z^nA
//
// -----------------------------------------------------------------------------
void gfPolEvalSeq(
	gfExp*	A,	// polynomial
	int		nA,	// deg(A) !! limited to [0 ... GF_N-1[  !!
	gfVec*	Yv,	// result array (vector repr.) with Yv[nY] = A(x) ... Yv[0] = A(x * z^nY)
	int		nY,	// number of locations - 1 (or deg(Y)) (min: 0)
	gfExp	x)	// 1st location
// -----------------------------------------------------------------------------
{
	dprintf("---------- polEvalSeq\n");
	PRINTPOL("seq: A", A, nA);
	// initialize all Yv with a0:
	gfVec a0 = gfE2V[A[0]];
	for (int iy=nY; iy>=0; iy--) {
		Yv[iy] = a0;
	}
	gfExp xi = x;								// start with xi = x^1
	for (int ia=1; ia<=nA; ia++) {				// ia = 1..nA
		// This line limits max(nA) to GF_N - 1:
		// Use expensive GF__Z(), if longer polynomials are needed.
		gfExp zi = GF_Z(ia);					// zi = z^ia
		// compute Yv[iy] += A[ia] * x * z^l:
		gfExp ai = A[ia];						// ai = A[ia]
		if (ai == GF_0)
			goto next_a;
		gfExp v = gfMul11(ai, xi);				// A[ia] * x^ia
		for (int iy=nY; iy>0; iy--) {			// iy = nY..0   |   l = 0..nY
			Yv[iy] ^= gfE2V[v];
			v = gfMul11(v, zi);					// v = A[ai] * x^ia * (z^ia)^l
		}
		Yv[0] ^= gfE2V[v];
next_a:
		xi = gfMul11(xi, x);					// xi = x^ia
	}
	PRINTPOL("seq: Yv", Yv, nY);
}


// -----------------------------------------------------------------------------
// Polynomial division A = B * Q + R with unknown lower coeffs.  Used by the
// EEA.
//
// Example:
// nA' = 6      nB' = 4 (incl. unknown lower coeffs)
// nA  = 3      nB  = 3 (excl. unknown lower coeffs)
// -> nQ = nA' - nB' = 2
// number of steps s = nQ + 1 = 3
// nR <= nA - s = nB - s = 0
// * means unknown, x is known (= q*b)
//
// r3 r2 r1 r0 *  *  * : b3 b2 b1 b0 * = q2 q1 q0
// x  x  x  x  *
// -------------
//    r2 r1 r0 *  *
//    x  x  x  x  *
//    -------------
//       r1 r0 *  *  *
//       x  x  x  x  *
//       -------------
//          r0 *  *  *
//
// ! only non-* values are passed in and out.
// -----------------------------------------------------------------------------
gfExp* gfPolDivEEA(
	gfExp*	A,	// in: numerator		! all without unknown coeffs !
	int		nA,	// in: max. deg(A)		!
	int		*nR,// out: max deg(R)		!
	gfExp*	B,	// in: denominator		!
	int		nB,	// in: actual deg(B)	!
	gfExp*	Q,	// out: quotient		!
	int		nQ)	// in: max. deg(Q) (steps - 1)
// -----------------------------------------------------------------------------
{
	dprintf("---------- polDivEEA\n");
	PRINTPOL("div: A", A, nA);
	PRINTPOL("div: B", B, nB);
	dprintf("div: nQ = %d\n", nQ);

	// OPT: use *(R--), *(Q--) etc. where meaningful
	gfExp b = B[nB];

	gfExp* R = A + (nA - nB);	// use only upper part
	gfVec* Rv = R;
	gfPolE2V(R, Rv, nB);			// convert to vector format
	*nR = nB;					// to be reduced with each step
	for (int iq = nQ; iq>=0; iq--) {	// nQ + 1 steps
		PRINTPOL("di1: Rv", Rv, *nR);
		gfExp r = gfV2E[Rv[*nR]];
		(*nR)--;
		if (r == GF_0) {
			Q[iq] = GF_0;
			continue;
		}
		gfExp q = gfDiv1(r, b);
		Q[iq] = q;

		int ib = nB-1;
		for (int ir = *nR; ir>=0; ir--) {
			Rv[ir] ^= gfE2V[gfMul01(B[ib--], q)];
		}
	}

	// convert result back to exponent representation:
	gfPolV2E(Rv, R, *nR);

	PRINTPOL("div: R", R, *nR);
	PRINTPOL("div: Q", Q, nQ);

	return R;
}


// -----------------------------------------------------------------------------
// Extended Euclidean algorithm
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
				// out: actual deg(Q)	// FIXME: check if needed/useful
	gfExp*	M)	// free memory of size: 7 * (nN+1) + 3
// -----------------------------------------------------------------------------
{
	dprintf("---------- polEEA\n");
	PRINTPOL("EEA: N", N, nN);
	PRINTPOL("EEA: A", A, nA);
	dprintf("EEA: nQ=%d\n", *nQ);
	// -------------------- init: --------------------
	// internal vars use external memory array:
	// OPT: merge vars where possible (e.g. use C/D as TQ/R (?))
	// OPT: identify tighter memory limits
	// !! remember that polDiv() uses R[-1] !!
	gfExp* Msave = M;	// test only
	int nTQ;		gfExp* TQ = M;		M += 2 + nN;	// quotient  of R2/R1
	int nR1 = nA;	gfExp* R1 = M;		M += 2 + nN;
	int nR2 = nN;	gfExp* R2 = M;		M += 1 + nN;
	int nC;			gfExp* C  = M;		M += 1 + nN;	// OPT: check act. max degrees of C, D
	int nC1;		gfExp* C1 = P;	// use P as tmp space
	int nC2;		gfExp* C2 = M;		M += 1 + nN;
	int nD;			gfExp* D  = M;		M += 1 + nN;
	int nD1;		gfExp* D1 = Q;	// use Q as tmp space
	int nD2;		gfExp* D2 = M;		M += 1 + nN;
	int nt;			gfExp* t;	// tmp
	// set R1=A, R2=N, C1=0, C2=1, D1=1, D2=0:
	// OPT: merge loops (check nC, nD)
	for (int i=nN; i>nA; i--)			// R2 = N
		R2[i] = N[i];
	for (int i=nA; i>=0; i--) {
		R1[i] = A[i];					// R1 = A
		// R2[i] = GF_0;				// R2 = N OPT: we know that N[i] = 0 here
		R2[i] = N[i];					// R2 = N
	}
	C1[0] = GF_0;	nC1 = -1;			// C1 = 0
	C2[0] = GF_1;	nC2 = 0;			// C2 = 1
	D1[0] = GF_1;	nD1 = 0;			// D1 = 1
	D2[0] = GF_0;	nD2 = -1;			// D2 = 0
	// -------------------- loop: --------------------
	nTQ = *nQ;
	while (1) {					// while (R1 != 0)
		PRINTPOL("EEA1: R2", R2, nR2);
		PRINTPOL("EEA1: R1", R1, nR1);

		R2 = gfPolDivEEA(R2, nR2, &nR2, R1, nR1, TQ, nTQ);	// R = R2 % R1

		PRINTPOL("EEA2: R2", R2, nR2);

		nD = gfPolMul(D1, nD1, TQ, nTQ, D);		// D = D1 * TQ
		nD = gfPolAdd(D, nD, D2, nD2, D);			// D += D2
		t = D2;
		D2 = D1;	nD2 = nD1;
		D1 = D;		nD1 = nD;
		D = t;

		nC = gfPolMul(C1, nC1, TQ, nTQ, C);		// C = C1 * TQ
		nC = gfPolAdd(C, nC, C2, nC2, C);			// C += C2
		t = C2;
		C2 = C1;	nC2 = nC1;
		C1 = C;		nC1 = nC;
		C = t;
		PRINTPOL("EEA: C", C1, nC1);
		PRINTPOL("EEA: D", D1, nD1);

		// R = R2 % R1 (at this time R is already stored in R2)
		// adapt degrees of R, TQ; done if (R=0):
		// (R will become R1, next denominator)
		nTQ = 1;	// correct for max. deg(R)
		while ((nR2 >= 0) && (R2[nR2] == GF_0)) {
			nR2--;
			nTQ++;	// deg(R1')-- => deg(TQ) = deg(R2' / R1')++
		}
		dprintf("1 - nTQ=%d\n", nTQ);
		dprintf("1 - nR=%d\n", nR2);
		if (nR2 < 0)	// R = 0 -> finished
			break;

		// swap R1 <-> R2:
		t  = R2;	nt  = nR2;	// old remainder
		R2 = R1;	nR2 = nR1;	// new numerator = old denominator
		R1 = t;		nR1 = nt;	// new denominator = old remainder
	}
	// copy P=C1, Q=D1, if neccessary.  We used rotating pointers C, C1, C2
	// above, so P may already point to C1 by incident (same for Q and D, D1,
	// D2).
	*nP = nC1;
	*nQ = nD1;
	if (P != C1) {					// compare pointers (if P=C then also Q=D)
		for (int i=nC1; i>=0; i--)
			P[i] = C1[i];
		for (int i=nD1; i>=0; i--)
			Q[i] = D1[i];
	}
	PRINTPOL("EEA: P", P, *nP);
	PRINTPOL("EEA: Q", Q, *nQ);
}


// -----------------------------------------------------------------------------
// Evaluate derivation A'(X) at X=x, x!=0
// In GF(2^N), A'(X) computes to:
//   A(X)  = sum(ai * X^i),     i=0,1,2,3,4,..., deg(A)
//   A'(X) = sum(ai * X^(i-1)), i=  1,  3,  ..., deg(A) (odd coeffs with even powers)
// Horner scheme:
//   an * X^n + a_{n-1} * X^{n-1} + ... + a2 * X^2 + a1 * X + a0 =
//   ((..((a_n)X + a_{n-1})X + ...)X + a1)X + a0
// -----------------------------------------------------------------------------
gfExp gfPolEvalDeriv(
	gfExp*	A,	// polynomial
	int		nA,	// max. deg(A)
	gfExp	x)	// location
// -----------------------------------------------------------------------------
{
	// using the Horner scheme.
	// select highest odd coeff:
	int i = nA;
	if ((i & 1) == 0)	// even
		i--;
	if (i < 0)
		return GF_0;
	gfExp x2 = gfMul11(x, x);	// x^2	// assume x!=0
	gfExp r = A[i];
	while (i >= 3) {
		i -= 2;
		r = gfMul01(r, x2);		// * x^2
		gfExp a = A[i];
		r = gfAdd(r, a);
	}
	PRINTPOL("evl: A", A, nA);
	dprintf("A'(%d)=%d\n", x, r);
	return r;
}

