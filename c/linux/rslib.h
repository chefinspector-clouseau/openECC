/*
 * include/linux/rslib.h
 *
 * Overview:
 *   Generic Reed Solomon encoder / decoder library
 *
 * Copyright (C) 2004 Thomas Gleixner (tglx@linutronix.de)
 *
 * RS code lifted from reed solomon library written by Phil Karn
 * Copyright 2002 Phil Karn, KA9Q
 *
 * $Id: rslib.h,v 1.4 2005/11/07 11:14:52 gleixner Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _RSLIB_H_
#define _RSLIB_H_

#include <ecc_cfg.h>	// GF_N, RS_N, RS_K, RS_N_K

// hacks to compile standalone:
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#define ERANGE 5
#define EBADMSG 6
#define min(a, b) (((a) < (b)) ? (a) : (b))

/**
 * struct rs_control - rs control structure
 *
 * @mm:		Bits per symbol
 * @nn:		Symbols per block (= (1<<mm)-1)
 * @alpha_to:	log lookup table
 * @index_of:	Antilog lookup table
 * @genpoly:	Generator polynomial
 * @nroots:	Number of generator roots = number of parity symbols
 * @fcr:	First consecutive root, index form
 * @prim:	Primitive element, index form
 * @iprim:	prim-th root of 1, index form
 * @gfpoly:	The primitive generator polynominal
 * @gffunc:	Function to generate the field, if non-canonical representation
 * @users:	Users of this structure
*/
struct rs_control {
	int 		mm;
	int 		nn;
	uint16_t	*alpha_to;
	uint16_t	*index_of;
	uint16_t	*genpoly;
	int 		nroots;
	int 		fcr;
	int 		prim;
	int 		iprim;
	int		gfpoly;
	int		(*gffunc)(int);
	int		users;
};

/* General purpose RS codec, 8-bit data width, symbol width 1-15 bit  */
#ifdef CONFIG_REED_SOLOMON_ENC8
int encode_rs8(struct rs_control *rs, uint8_t *data, int len, uint16_t *par,
	       uint16_t invmsk);
#endif
#ifdef CONFIG_REED_SOLOMON_DEC8
int decode_rs8(struct rs_control *rs, uint8_t *data, uint16_t *par, int len,
		uint16_t *s, int no_eras, int *eras_pos, uint16_t invmsk,
	       uint16_t *corr);
#endif

/* General purpose RS codec, 16-bit data width, symbol width 1-15 bit  */
#ifdef CONFIG_REED_SOLOMON_ENC16
int encode_rs16(struct rs_control *rs, uint16_t *data, int len, uint16_t *par,
		uint16_t invmsk);
#endif
#ifdef CONFIG_REED_SOLOMON_DEC16
int decode_rs16(struct rs_control *rs, uint16_t *data, uint16_t *par, int len,
		uint16_t *s, int no_eras, int *eras_pos, uint16_t invmsk,
		uint16_t *corr);
#endif

/* Create or get a matching rs control structure */
int init_rs(struct rs_control *rs, int symsize, int gfpoly, int fcr, int prim,
			   int nroots);
int init_rs_non_canonical(struct rs_control *rs, int symsize, int (*func)(int),
                                         int fcr, int prim, int nroots);

/* Release a rs control structure */
void free_rs(struct rs_control *rs);

/** modulo replacement for galois field arithmetics
 *
 *  @rs:	the rs control structure
 *  @x:		the value to reduce
 *
 *  where
 *  GF_NLOG = number of bits per symbol
 *  (GF_N - 1) = (2^GF_NLOG) - 1
 *
 *  Simple arithmetic modulo would return a wrong result for values
 *  >= 3 * (GF_N - 1)
*/
static inline int rs_modnn(struct rs_control *rs, int x)
{
	while (x >= (GF_N - 1)) {
		x -= (GF_N - 1);
		x = (x >> GF_NLOG) + (x & (GF_N - 1));
	}
	return x;
}

#endif
