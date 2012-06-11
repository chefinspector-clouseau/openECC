// -----------------------------------------------------------------------------
// Test functions and application example for rs.c
//
// Copyright (C) 2012 Till Schmalmack
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#include "test_util.h"		// random + compare functions
#include <rs/rs.h>			// rsEncode() / rsDecode()
#include <gf/gf.h>			// gfExp type
#include <stdlib.h>			// atoi (cmdline parsing)

#ifndef DEBUG_TEST			// disable debug output for this file
  #define printf
  #define printPol
#endif

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

	printf("RS: --------------------\n");

	// ---------- random info word: ----------
	randPol(A, RS_K - 1);
	printPol("RS:  A", A, RS_K - 1);

	// ---------- encode: ----------
	for (int i=0; i<nEnc; i++)	// speed test
		rsEncode(A, R);
	// copy R back into C:
	for (int i=0; i<RS_N_K; i++)
		C[i] = R[i];
	printPol("RS:  C", C, RS_N - 1);

	// ---------- pick error: ----------
	static gfExp EV[RS_N];		// error vector
	for (int i=0; i<RS_N; i++)
		EV[i] = GF_0;
	printf("RS: nErrs = %d\n", nErrs);
	for (int i=0; i<nErrs; i++) {
		int loc;	// find not-yet-used error location
		do {
			loc = randInt(0, RS_N - 1);
		} while (EV[loc] != GF_0);	// already used -> try again
		int value = randE1();		// error must be non-zero
		EV[loc] = value;
		//printf("RS: EV[%d] = %d\n", loc, value);
	}
	printPol("RS: EV", EV, RS_N - 1);

	// ---------- add error: ----------
	gfPolAdd(C, RS_N - 1, EV, RS_N - 1, C2);
	printPol("RS: C2", C2, RS_N - 1);
	for (int i=0; i<nDec; i++) {			// speed test
		// restore erroneous code word (should not be part of the speed test but
		// required here):
		for (int k=0; k<RS_N; k++)
			C3[k] = C2[k];
		// ---------- decode + correct: ----------
		rsDecode(C3);
	}
	gfExp* A2 = C3 + RS_N_K;				// corrected user data
	printPol("RS: A2", A2, RS_K - 1);

	// ---------- verify: ----------
	if (! polCmp(A, A2, RS_K - 1, RS_K - 1))
	{
		printPol("RS: A ", A,  RS_K - 1);
		printPol("RS: A2", A2, RS_K - 1);
		return 1;
	}
	return 0;
}


// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
// -----------------------------------------------------------------------------
{
	// arg parsing:
	argc--; argv++;
	// defaults:
	int nRuns = 1;			// # of test runs
	int nErrs = RS_N_K/2;	// # of errors to inject (-1 for random [0..max])
	int seed  = 1;			// random seed
	int nEnc  = 1;			// encode # times (speed test)
	int nDec  = 1;			// decode # times (speed test)
	if (argc-- > 0) nRuns = atoi(*(argv++));
	if (argc-- > 0) nErrs = atoi(*(argv++));
	if (argc-- > 0) seed  = atoi(*(argv++));
	if (argc-- > 0) nEnc  = atoi(*(argv++));
	if (argc-- > 0) nDec  = atoi(*(argv++));

	randSetSeed(seed);

	rsInit();

	for (int run = 0; run < nRuns; run++) {
		int nErrs2;
		if (nErrs >= 0)
			nErrs2 = nErrs;
		else								// random # of errors
			nErrs2 = randInt(0, RS_N_K/2);
		if (rsTest(nErrs2, nEnc, nDec))
			return 1;						// abort on error
	}
	return 0;
}
