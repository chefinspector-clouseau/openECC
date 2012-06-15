// =============================================================================
// Reed-Solomon configuration:
// =============================================================================

#ifndef _ECC_CFG_RS_H
#define _ECC_CFG_RS_H

// -----------------------------------------------------------------------------
// user params:
// -----------------------------------------------------------------------------

// Symbol size in bits:
#ifndef GF_NLOG
  #define GF_NLOG 4
#endif

#define GF_N (1 << (GF_NLOG))

// Codeword length (info + check part) in symbols.  If not defined, the maximum
// length of
//    GF_N - 1
// is used.
#ifndef RS_N
 #define RS_N (GF_N - 1)
#endif

// Number of redundant check symbols per codeword (min: 1):
#ifndef RS_N_K			// N-K
  #define RS_N_K 4
#endif


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
