// -----------------------------------------------------------------------------
// Hack: some wrappers to adapt linux RS API to our API:
// -----------------------------------------------------------------------------

#include <ecc_cfg.h>	// GF_N, RS_N, RS_K, RS_N_K
#include <gf/gf.h>
#include "rslib.h"
#include <string.h>		// memset()

int primPols[] = {
	0,	// GF(1)
	0,	// GF(2)
	0x0003,	//                  11 (1): (x^2) + x + 1
	0x0006,	//                 110 (1): (x^3) + x + 1
	0x000c,	//                1100 (1): (x^4) + x + 1
	0x0014,	//              1 0100 (1): (x^5) + x^2 + 1
	0x0030,	//             11 0000 (1): (x^6) + x + 1
	0x0044,	//            100 0100 (1): (x^7) + x^4 + 1
	0x00b8,	//           1011 1000 (1): (x^8) + x^4 + x^3 + x^2 + 1
	0x0110,	//         1 0001 0000 (1): (x^9) + x^4 + 1
	0x0240,	//        10 0100 0000 (1): (x^10) + x^3 + 1
	0x0500,	//       101 0000 0000 (1): (x^11) + x^2 + 1
	0x0ca0,	//      1100 1010 0000 (1): (x^12) + x^6 + x^4 + x + 1
	0x1b00,	//    1 1011 0000 0000 (1): (x^13) + x^4 + x^3 + x + 1
	0x3500,	//   11 0101 0000 0000 (1): (x^14) + x^5 + x^3 + x + 1
	0x6000,	//  110 0000 0000 0000 (1): (x^15) + x + 1
	0xb400,	// 1011 0100 0000 0000 (1): (x^16) + x^5 + x^3 + x^2 + 1
};
#define GFPOLY ((primPols[GF_NLOG] << 1) | 1)

static uint16_t alpha_to[GF_N];
static uint16_t index_of[GF_N];
static uint16_t genpoly[RS_N_K + 1];
static struct rs_control rs;

// -----------------------------------------------------------------------------
void rsInit()
// -----------------------------------------------------------------------------
{
	rs.alpha_to = alpha_to;
	rs.index_of = index_of;
	rs.genpoly = genpoly;
	init_rs(
		&rs,
		GF_NLOG,		// symsize
		GFPOLY,			// gfpoly
		1,				// fcr
		1,				// prim
		RS_N_K);		// nroots
}

// -----------------------------------------------------------------------------
void rsEncode(gfExp* A, gfExp* R)
// -----------------------------------------------------------------------------
{
	memset(R, 0, RS_N_K * sizeof(gfExp));
	encode_rs16(&rs, A, RS_K, R, 0);
}


// -----------------------------------------------------------------------------
void rsDecode(gfExp* C)
// -----------------------------------------------------------------------------
{
// 	static uint16_t corr[RS_N];
// 	static int err_pos[RS_N];
	gfExp* data = C + RS_N_K;
	gfExp* par  = C;
//	memset(corr, 0, RS_N * sizeof(corr[0]));
	decode_rs16(&rs, data, par, RS_K, 0, 0, 0, 0, 0);
// 	// ---------- apply correction:
// 	for (int i=0; i<RS_K; i++)
// 		A[i] = data[i];
// 	for (int i=0; i<nErrs; i++) {
// 		int pos = err_pos[i];
// 		A[pos] ^= corr[i];
// 	}
}
