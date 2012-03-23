// =============================================================================
// BCH configuration:
// =============================================================================

#ifndef _ECC_CFG_BCH_H
#define _ECC_CFG_BCH_H

// -----------------------------------------------------------------------------
// user params:
// -----------------------------------------------------------------------------

#error "BCH not yet implemented."

// Codeword length (info + check part) in bits:
// (usually 2^n - 1)
#define BITS_PER_CODEWORD 15

// Number of bits that can be corrected:
// (Generator polynomial will have 2x that many *consecutive* roots; degree not yet known)
#define CORRECTABLE_BITS 2




#if (BITS_PER_CODEWORD < 8)
 #define GF_N 8
#elif (BITS_PER_CODEWORD < 16)
 #define GF_N 16
#elif (BITS_PER_CODEWORD < 32)
 #define GF_N 32
#elif (BITS_PER_CODEWORD < 64)
 #define GF_N 64
#elif (BITS_PER_CODEWORD < 128)
 #define GF_N 128
#elif (BITS_PER_CODEWORD < 256)
 #define GF_N 256
#elif (BITS_PER_CODEWORD < 512)
 #define GF_N 512
#elif (BITS_PER_CODEWORD < 1024)
 #define GF_N 1024
#elif (BITS_PER_CODEWORD < 2048)
 #define GF_N 2048
#else
 #error "invalid config"
#endif

#define BCH_N BITS_PER_CODEWORD

// TODO

#endif // _ECC_CFG_BCH_H
