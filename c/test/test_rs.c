// -----------------------------------------------------------------------------
// Test functions and application example for rs.c
//
// Copyright (C) 2012 Till Schmalmack
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#include "test_util.h"
#include <rs/rs.h>
#include <gf/gf.h>
#include <stdlib.h>	// atoi

// encode, add error, decode.
// return 0 for success
// -----------------------------------------------------------------------------
int rsTest(
	int nErrs,	// number of errors to inject
	int nEnc,	// encode # times (for speed test)
	int nDec)	// decode # times
// -----------------------------------------------------------------------------
{
	static gfExp C[RS_N];		// code word
	gfExp* A = C + RS_N_K;		// encoder needs A[-1] .. A[-(n-k)]

	static gfExp R_[RS_N_K + 1];// n-k check symbols (+1)
	gfExp* R = R_ + 1;			// encoder needs R[-1]

	static gfExp C2[RS_N];		// code word with errors
	static gfExp C3[RS_N];		// code word with errors, used in loop

	dprintf("RS: --------------------\n");

	// ---------- random info word: ----------
	randPol(A, RS_K - 1);
	PRINTPOL("RS:  A", A, RS_K - 1);

	// ---------- encode: ----------
	for (int i=0; i<nEnc; i++)	// speed test
		rsEncode(A, R);
	// copy R back into C:
	for (int i=0; i<RS_N_K; i++)
		C[i] = R[i];
	PRINTPOL("RS:  C", C, RS_N - 1);

	// ---------- pick error: ----------
	static gfExp EV[RS_N];		// error vector
	for (int i=0; i<RS_N; i++)
		EV[i] = GF_0;
	dprintf("RS: nErrs = %d\n", nErrs);
	for (int i=0; i<nErrs; i++) {
		int loc;	// find not-yet-used error location
		do {
			loc = randInt(0, RS_N - 1);
		} while (EV[loc] != GF_0);	// already used -> try again
		int value = randE1();		// error must be non-zero
		EV[loc] = value;
		//dprintf("RS: EV[%d] = %d\n", loc, value);
	}
	PRINTPOL("RS: EV", EV, RS_N - 1);

	// ---------- add error: ----------
	gfPolAdd(C, RS_N - 1, EV, RS_N - 1, C2);
	PRINTPOL("RS: C2", C2, RS_N - 1);
	for (int i=0; i<nDec; i++) {			// speed test
		// restore erroneous code word (should not be part of the speed test but
		// required here):
		for (int k=0; k<RS_N; k++)
			C3[k] = C2[k];
		// ---------- decode + correct: ----------
		rsDecode(C3);
	}
	gfExp* A2 = C3 + RS_N_K;				// corrected user data
	PRINTPOL("RS: A2", A2, RS_K - 1);

	// ---------- verify: ----------
	if (! polCmp(A, A2, RS_K - 1, RS_K - 1))
	{
		PRINTPOL("RS: A ", A,  RS_K - 1);
		PRINTPOL("RS: A2", A2, RS_K - 1);
		return 1;
	}
	return 0;
}


#ifndef TEST_RUNS
  #define TEST_RUNS 1	// demo only
#endif

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
// -----------------------------------------------------------------------------
{
	// arg parsing:
	argc--; argv++;
	// defaults:
	int nErrs = RS_N_K/2;
	int nEnc = 1;
	int nDec = 1;
	if (argc-- > 0) nErrs = atoi(*(argv++));
	if (argc-- > 0) nEnc  = atoi(*(argv++));
	if (argc-- > 0) nDec  = atoi(*(argv++));

	rsInit();

	for (int test=0; test<TEST_RUNS; test++) {
		if (rsTest(nErrs, nEnc, nDec))
			return 1;
	}
	return 0;
}
