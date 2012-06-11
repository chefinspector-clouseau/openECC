// -----------------------------------------------------------------------------
// Simple test helpers, incl. very simple PRNG ("quick")
//
// Copyright (C) 2012 Till Schmalmack
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#include "test_util.h"

unsigned int SEED = 1;

// set random seed
// -----------------------------------------------------------------------------
void randSetSeed(unsigned int seed)
{
	SEED = seed;
}

// return random int in [min, max]
// -----------------------------------------------------------------------------
unsigned int randInt(unsigned int min, unsigned int max)
// -----------------------------------------------------------------------------
{
	SEED = (1099087573 * SEED) & 0xffffffff;
	return min + ((SEED>>8) % (max - min + 1));
}

// return random element of GF:
// -----------------------------------------------------------------------------
gfExp randE()
// -----------------------------------------------------------------------------
{
	return randInt(0, GF_N - 1);
}

// return random nonzero element of GF:
// -----------------------------------------------------------------------------
gfExp randE1()
// -----------------------------------------------------------------------------
{
	return randInt(GF_1, GF_N - 1);
}


// fill random polynomial with given max. degree
// -----------------------------------------------------------------------------
void randPol(gfExp* P, int nP)
// -----------------------------------------------------------------------------
{
	for (int i = 0; i<=nP; i++)
		P[i] = randE();
}


// return 1 if A==B
// -----------------------------------------------------------------------------
int polCmp(gfExp* A, gfExp* B, int nA, int nB)
// -----------------------------------------------------------------------------
{
	gfExp* S;	int nS;				// shorter
	gfExp* L;	int nL;				// longer
	if (nA > nB) {
		L = A; nL = nA; S = B; nS = nB;
	} else {
		L = B; nL = nB; S = A; nS = nA;
	}
	for (int i=0; i<=nS; i++)
		if (S[i] != L[i])
			return 0;
	for (int i=nS+1; i<=nL; i++)
		if (L[i] != GF_0)
			return 0;
	return 1;
}
