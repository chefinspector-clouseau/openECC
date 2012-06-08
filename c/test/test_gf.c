// -----------------------------------------------------------------------------
// Test functions for gf.c
//
// Copyright (C) 2012 Till Schmalmack
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#include <stdio.h>
#include "test_util.h"
#include <gf/gf.h>

// -----------------------------------------------------------------------------
int main ()
// -----------------------------------------------------------------------------
{
	// verify basic operations (mul, add, div):
	// a+b = b+a
	// a*b = b*a
	// a+(b+c) = (a+b)+c
	// a*(b*c) = (a*b)*c
	// a*c + b*c = (a+b)*c
	// a*(b/a) = b (if a != 0)
	gfInit();

	for (int a=0; a<GF_N; a++) {
		for (int b=0; b<GF_N; b++) {
			if (gfAdd(a, b) != gfAdd(b, a))
				return 1;
			if (gfMul(a, b) != gfMul(b, a))
				return 2;
			if (gfMul(a, b) != gfMul(b, a))
				return 2;
			for (int c=0; c<GF_N; c++) {
				if (gfAdd(a, gfAdd(b, c)) != gfAdd(gfAdd(a, b), c))
					return 3;
				if (gfMul(a, gfMul(b, c)) != gfMul(gfMul(a, b), c))
					return 4;
				if (gfAdd(gfMul(a, c), gfMul(b, c)) != gfMul(gfAdd(a, b), c))
					return 5;
			}
			if (a != GF_0)
				if (gfMul(a, gfDiv(b, a)) != b)
					return 6;
		}
	}

	// verify polynomial operations:
	// A = BQ + R
	const int M = GF_N;	// max deg
	gfExp  A[M+1];		int nA;
	gfExp  B[M+1];		int nB;
	gfExp  Q[M+1];		int nQ;
	gfExp  R1[M+2];
	gfExp* R = R1 + 1;	int nR;
	gfExp  Z[M+1];		int nZ;
	gfExp  Z1[2*M];		int nZ1;
	gfExp  Z2[2*M];		int nZ2;
	gfExp* P = R;		int nP;	// alias
	gfExp* N = B;		int nN;	// alias
	gfExp* Y = Q;		int nY;	// alias
	gfExp  Mem[8*(M+1) + 3];

// 	// -------------------- test polDiv, polMul, gfPolAdd(): --------------------
// 	for (int test=0; test<100000; test++) {
// 		// // clear all -- should not be required:
// 		// for (int i=0; i<=M; i++)
// 		// 	A[i] = B[i] = Q[i] = R[i] = Z[i] = 0;
// 		// R[-1] = 0;

// 		nB = randInt(0, M);
// 		nA = randInt(nB, M);
// 		randPol(A, nA);
// 		randPol(B, nB);
// 		if (gfPolDeg(B, nB) == -1)		// avoid dividing by B=0
// 			continue;
// 		nB = polDiv(A, nA, B, nB, Q, &nQ, R, &nR);
// 		nZ = gfPolMul(Q, nQ, B, nB, Z);				// B * Q
// 		if (nZ > nA)
// 			return 7;
// 		nZ = gfPolAdd(Z, nZ, R, nR, Z);				//       + R
// 		if (! polCmp(Z, A, nZ, nA))
// 			return 8;
// 	}

// 	// -------------------- test gfPolEEA(): --------------------
// 	for (int test=0; test<100000; test++) {
// 		nN = randInt(1, M);
// 		nA = randInt(0, nN-1);
// 		randPol(A, nA);
// 		randPol(N, nN);
// 		if (gfPolDeg(A, nA) == -1)		// avoid dividing by A=0
// 			continue;
// 		gfPolEEA(N, nN, A, nA, P, &nP, Q, &nQ, Mem);
// 		nZ1 = gfPolMul(P, nP, N, nN, Z1);
// 		nZ2 = gfPolMul(Q, nQ, A, nA, Z2);
// 		if (! polCmp(Z1, Z2, nZ1, nZ2))
// 			return 9;
// 	}

	// -------------------- test gfPolEvalSeq() against gfPolEval(): --------------------
	for (int test=0; test<10000; test++) {
		nA = randInt(0, GF_N - 2);	// limit of gfPolEvalSeq()
		nY = randInt(0, M);
		gfVec* Yv = Y;
		randPol(A, nA);
		gfExp x = GF_Z(1);//randE1();
		gfPolEvalSeq(A, nA, Yv, nY, x);
		for (int i=0; i<=nY; i++) {
			gfExp y2 = gfPolEval(A, nA, x);
			gfExp y1 = gfV2E[Yv[nY-i]];
			if (y2 != y1)
				return 10;
			x = gfMul(x, GF_Z(1));
		}
	}

	return 0;
}
