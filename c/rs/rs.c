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

#include "rs.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Generator polynomial (aka D(X)).  It has n-k roots at the consecutive
// locations z^1 ... z^(n-k) (narrow-sense BCH code).
static gfExp rsGen[RS_N_K + 1];

// Super polynomial N(X) = X^m - 1 (m = GF_N-1).  It has roots at all z^i,
// i=0..m-1.  In GF(16): N(X) = X^15 - 1 = prod(X-z^i) for i = 0..14.  We save
// only the highest n-k coefficients.
static gfExp rsSup[RS_N_K];

// Compute generator polynomial and super polynomial
// (may instead be done at compile time):
// rsGen(X) = prod(X - z^i) for i = 1 ... n-k
// rsSup(X) = prod(X - z^i) for i = 0 ... m-1
//          = X^m - 1
// -----------------------------------------------------------------------------
void rsInit()
// -----------------------------------------------------------------------------
{
	dprintf("---------- rsInit\n");

	gfInit();

	// ---------- compute rsSup = X^m - 1 (only upper n-k coeffs)
	rsSup[RS_N_K - 1] = GF_1;
	for (int i=RS_N_K - 2; i>=0; i--)
		rsSup[i] = GF_0;

	// ---------- compute rsGen:
	gfExp T[RS_N_K + 1];	// tmp
	int nD1 = 0;	gfExp* D1 = rsGen;
	int nD2;		gfExp* D2 = T;
	D1[0] = GF_1;
	gfExp Z[2] = {GF_Z(1), 1};			// 1st root of rsGen(X): (X - z^1)
	for (int i=RS_N_K; i>0; i--) {
		nD2 = gfPolMul(D1, nD1, Z, 1, D2);
		Z[0]++;						// (X - z^i) => (X - z^(i+1))
		// swap D1 <-> D2:
		int    s = nD1;
		gfExp* t =  D1;
		D1 = D2;	nD1 = nD2;
		D2 = t;		nD2 = s;
		// result is in D1
	}
	// copy rsGen = D1 (if not already identical)
	for (int i=nD1; i>=0; i--) {
		// an optimization in gfPolDiv1() requires (rsGen[i] != 0 for all i)
		// -> check here
		if (D1[i] == GF_0) {
			dprintf("!! BAD CONFIG: generator polynomial has 0-coefficient !!\n");
			rsGen[0] = 1 / rsSup[0];	// throw DBZ-exception (trying to fool compiler)
		}
		rsGen[i] = D1[i];
	}

	PRINTPOL("ini: rsSup", rsSup, RS_N_K - 1);
	PRINTPOL("ini: rsGen", rsGen, RS_N_K);
}


// Compute check symbols from information symbols.
// Compute R(X) = (X^(n-k) * A(X)) % D(X)
// Codeword is then C = (A, R)
// deg(A) <= k-1, deg(R) <= n-k-1
// !! Abuses some memory at negative indices of A and R, see below !!
// !! A and R must NOT overlap !!
// -----------------------------------------------------------------------------
void rsEncode(
	gfExp* A,	// !! A[k-1]   ... A[0]   A[-1] ... A[-(n-k)] !!
	gfExp* R)	// !! R[n-k-1] ... R[0]   R[-1]               !!
				// !! <-- user part -->   <---- abused  ---->
// -----------------------------------------------------------------------------
{
	dprintf("---------- rsEncode\n");
	PRINTPOL("enc: A", A, RS_K - 1);
	gfExp* XA = A - RS_N_K;				// X^(n-k) * A(X)
	// clear XA[0] .. XA[n-k-1]:
	for (int i=0; i<RS_N_K; i++)
		XA[i] = GF_0;
	gfPolDiv1(XA, RS_N - 1, rsGen, RS_N_K, R);
	PRINTPOL("enc: A", A, RS_K - 1);
	PRINTPOL("enc: R", R, RS_N_K - 1);
}


// Compute information word from code word, correcting up to n-k/2 errors.
// -----------------------------------------------------------------------------
void rsDecode(
	gfExp* C,	// in: codeword     C[n-1] ... C[n-k] C[n-k-1] ... C[0]
	gfExp* A)	// out: info word   A[k-1] ... A[0]
// -----------------------------------------------------------------------------
{
	dprintf("---------- rsDecode\n");
	PRINTPOL("dec: C", C, RS_N - 1);
	static gfVec Sv[RS_N_K];	// syndrome in vector representation
	// calculate syndrome using the DFT:
	gfPolEvalSeq(C, RS_N - 1, Sv, RS_N_K - 1, GF_Z(1));
	PRINTPOL("dec: Sv", Sv, RS_N_K - 1);
	#define MSIZE1 (7 * (RS_N_K + 1) + 3)	// for the EEA
	#define MSIZE2 (RS_K)					// to evaluate Q(X) at K locations
	static gfExp M[MAX(MSIZE1, MSIZE2)];	// memory
	static gfExp P[RS_N];	int nP;	// OPT: determine better limits for deg(P), deg(Q)
	static gfExp Q[RS_N];	int nQ;
	int nS = gfPolDeg(Sv, RS_N_K - 1);
	// If the syndrome is 0, we're done.  Else start the EEA:
	if (nS >= 0) {					// S != 0
		// full N'(X) (aka rsSup) has deg(N') = n
		// full syndrome S' has deg(S') <= n-1
		// We consider only the highest n-k coeffs of S' an N':
		// deg(N) = deg(N') - k - 1	= n-k-1
		// deg(S) = deg(S') - k
		// deg(S)                  <= n-k-1
		// for deg(S') = n-1  -> deg(S) = n-k-1  (full degree):
		//   deg(N'/S') = deg(N') - deg(S') = n - (n-1) = 1
		//   -> number of steps in 1st EEA div is 2
		// for lower degree: deg(S') = n-1-l    -> deg(S) = n-k-1-l
		//   -> deg(N'/S') = 1 + l
		//   l = n-k-1 - deg(S)
		//   -> deg(N'/S') = 1 + n-k-1 - deg(S) = n-k - deg(S)
		gfExp* S = Sv;
		gfPolV2E(Sv, S, nS);	// convert back to exp
		nQ = RS_N_K - nS;	// deg(N') - deg(S'), see above
		gfPolEEA(rsSup, RS_N_K - 1, S, nS, P, &nP, Q, &nQ, M);
		// Now we have:
		//	Q = prod(X-z^i), for all error locations i
		// -> find roots of Q that are in the information part n-k...n-1
		//    (we don't care for the others):
		gfVec* Vv = M;	// values of Q(z^i) (reuse memory M)
		gfPolEvalSeq(Q, nQ, Vv, RS_K - 1, GF_Z(RS_N_K));
		for (int i=RS_N-1; i>=RS_N_K; i--) {
			if (*(Vv++) == GF_0) {	// root at z^i => error at C[i]
				gfExp x = GF_Z(i);	// z^i
				// compute C[i] -= P(x) * N'(x) / Q'(x)
				// N(X) = X^m - 1, m=2^N-1, odd
				// N'(X) = X^(m-1) = X^(-1)
				// N'(x) = x^(-1)
				gfExp nx = gfInv1(x);	// N'(x) = x^(-1) != 0
				gfExp px = gfPolEval(P, nP, x);	// P(x) = E(x) / G(x) != 0 since E(x) != 0
				gfExp qx = gfPolEvalDeriv(Q, nQ, x);
				gfExp e = gfDiv1(gfMul11(px, nx), qx);
				C[i] = gfSub(C[i], e);
				dprintf("Q(%d) = 0 => error E(%d)=%d; new C[%d]=%d\n", x, i, e, i, C[i]);
			}
		}
	}
	for (int i=0; i<RS_K; i++)
		A[i] = C[i + RS_N_K];
	PRINTPOL("dec: A", A, RS_K - 1);
}
