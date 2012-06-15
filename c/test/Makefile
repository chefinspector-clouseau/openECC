# needs cleanup (if not concept...)

CFLAGS = -std=c99 -O3

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

ifeq ($(CODEC), linux)
  DEFS += -DGF_TYPE_SHORT
  RS-OBJ = ../linux/rs-linux.o ../linux/reed_solomon.o
  ../linux/rs-linux.o: FORCE
	$(MAKE) -C ../linux $(RS-OBJ)
else
  RS-OBJ = ../rs/rs.o
endif

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
	$(MAKE) -s -C ../linux clean
	rm -f test_gf
	rm -f test_rs
	rm -f *.o
