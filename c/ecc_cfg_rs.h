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
