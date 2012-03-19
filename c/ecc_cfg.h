// User parameters

#ifndef _ECC_CFG_H
#define _ECC_CFG_H

// Select ECC code from this list:
#define ECC_CODE_RS  1	// Reed-Solomon
#define ECC_CODE_BCH 2	// BCH: Not implemented yet!

#define ECC_CODE ECC_CODE_RS




// -----------------------------------------------------------------------------
// Reed-Solomon:
// -----------------------------------------------------------------------------
#if (ECC_CODE == ECC_CODE_RS)

 // Symbol size:
 #define BITS_PER_SYMBOL 4

 // Codeword length (info + check part).  If not defined, the maximum length of
 //  2 ^ BITS_PER_SYMBOL - 1
 // is used.
 // #define SYMBOLS_PER_CODEWORD 12		// custom value

 // Number of redundant check symbols per codeword (min: 1):
 #define CHECK_SYMBOLS_PER_CODEWORD 4

 #include "ecc_cfg_rs.h"




// -----------------------------------------------------------------------------
// BCH:
// -----------------------------------------------------------------------------
#elif (ECC_CODE == ECC_CODE_BCH)

 #error "BCH not yet implemented."

 // Codeword length (info + check part) in bits:
 // (usually 2^n - 1)
 #define BITS_PER_CODEWORD 15

 // Number of bits that can be corrected:
 // (Generator polynomial will have 2x that many *consecutive* roots; degree not yet known)
 #define CORRECTABLE_BITS 2

 #include "ecc_cfg_bch.h"

#endif

#endif	// _ECC_CFG_H
