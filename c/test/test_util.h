// -----------------------------------------------------------------------------
// Simple test helpers, incl. very simple PRNG ("quick")
//
// Copyright (C) 2012 Till Schmalmack
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
// -----------------------------------------------------------------------------

#ifndef _TEST_UTIL_H
#define _TEST_UTIL_H

#include <gf/gf.h>

// set random seed
// -----------------------------------------------------------------------------
void randSetSeed(unsigned int seed);


// return random int in [min, max]
// -----------------------------------------------------------------------------
unsigned int randInt(unsigned int min, unsigned int max);


// return random element of GF
// -----------------------------------------------------------------------------
gfExp randE();


// return random nonzero element of GF
// -----------------------------------------------------------------------------
gfExp randE1();


// fill random polynomial with given max. degree
// -----------------------------------------------------------------------------
void randPol(gfExp* P, int nP);


// return 1 if A==B
// -----------------------------------------------------------------------------
int polCmp(gfExp* A, gfExp* B, int nA, int nB);

#endif // _TEST_UTIL_H
