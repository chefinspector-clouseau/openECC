# needs cleanup (if not concept...)

CFLAGS = -std=c99 -O3

MAKEFLAGS += -r --no-print-directory

# symbol size in bits:
ifneq ($(GF_NLOG),)
  DEFS += -DGF_NLOG=$(GF_NLOG)
endif

# number of symbols per codeword (n):
ifneq ($(RS_N),)
  DEFS += -DRS_N=$(RS_N)
endif

# number of symbols to add (redundancy n-k):
ifneq ($(RS_N_K),)
  DEFS += -DRS_N_K=$(RS_N_K)
endif

# enable debug output in core GF functions:
ifneq ($(DEBUG_GF),)
  DEFS += -DDEBUG_GF
endif

# enable debug output in RS codec:
ifneq ($(DEBUG_RS),)
  DEFS += -DDEBUG_RS
endif

# enable debug output in test bench:
ifneq ($(DEBUG_TEST),)
  DEFS += -DDEBUG_TEST
endif

default : test_rs

.PHONY: FORCE

RS-OBJ = ../rs/rs.o

export DEFS

../gf/gf.o: FORCE
	$(MAKE) -C ../gf gf.o

../rs/rs.o: FORCE
	$(MAKE) -C ../rs rs.o

%.o: %.c %.h ../ecc_cfg.h Makefile
	$(CC) -o $@ $(DEFS) -c $(CFLAGS) -I.. $<

test_gf: test_gf.c test_util.o ../gf/gf.o
	$(CC) -o $@ $(DEFS) $(CFLAGS) -I.. ../gf/gf.o test_util.o $<


test_rs: test_rs.c test_util.o ../gf/gf.o $(RS-OBJ)
	$(CC) -o $@ $(DEFS) $(CFLAGS) -I.. ../gf/gf.o test_util.o $(RS-OBJ) $<

test: test_rs FORCE
	./test_rs; echo $$?

clean:
	$(MAKE) -s -C ../gf clean
	$(MAKE) -s -C ../rs clean
	rm -f test_gf
	rm -f test_rs
	rm -f *.o
