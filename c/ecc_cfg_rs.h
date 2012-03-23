// =============================================================================
// Reed-Solomon configuration:
// =============================================================================

#ifndef _ECC_CFG_RS_H
#define _ECC_CFG_RS_H

// -----------------------------------------------------------------------------
// user params:
// -----------------------------------------------------------------------------

// Symbol size:
#define BITS_PER_SYMBOL 4

// Codeword length (info + check part).  If not defined, the maximum length of
//  2 ^ BITS_PER_SYMBOL - 1
// is used.
// #define SYMBOLS_PER_CODEWORD 12		// custom value

// Number of redundant check symbols per codeword (min: 1):
#define CHECK_SYMBOLS_PER_CODEWORD 4





// -----------------------------------------------------------------------------
// derived params:
// -----------------------------------------------------------------------------

#define GF_N (1 << (BITS_PER_SYMBOL))

#ifndef SYMBOLS_PER_CODEWORD
 #define SYMBOLS_PER_CODEWORD (GF_N - 1)
#endif

// shortcuts:
#define RS_N (SYMBOLS_PER_CODEWORD)
#define RS_N_K (CHECK_SYMBOLS_PER_CODEWORD)
#define RS_K (RS_N - RS_N_K)

// sanity checks:
#if (RS_N >= GF_N)
  #error "invalid config"
#elif (RS_K <= 0)
  #error "invalid config"
#elif (RS_N_K <= 0)
  #error "invalid config"
#endif

#endif // _ECC_CFG_RS_H
