# needs cleanup (if not concept...)

CFLAGS = -std=c99 -O1

ifneq ($(DEBUG_ALL),)
  DEBUG_GF = 1
  DEBUG_RS = 1
  DEBUG_TEST = 1
endif

ifdef TEST_RUNS
  DEFS += -DTEST_RUNS=$(TEST_RUNS)
endif

ifneq ($(DEBUG_TEST),)
  DEFS += -DDEBUG
endif

all: test_gf test_rs

.PHONY: FORCE

../gf/gf.o: FORCE
	make DEBUG_GF=$(DEBUG_GF) -C ../gf gf.o

../rs/rs.o: FORCE
	make DEBUG_RS=$(DEBUG_RS) -C ../rs rs.o

%.o: %.c %.h ../ecc_cfg.h Makefile
	$(CC) -o $@ -c $(CFLAGS) -I.. $<

test_gf: test_gf.c test_util.o ../gf/gf.o
	$(CC) -o $@ $(DEFS) $(CFLAGS) -I.. ../gf/gf.o test_util.o $<

test_rs: test_rs.c test_util.o ../gf/gf.o ../rs/rs.o
	$(CC) -o $@ $(DEFS) $(CFLAGS) -I.. ../gf/gf.o ../rs/rs.o test_util.o $<

test: test_rs FORCE
	./test_rs; echo $$?

clean:
	make -s -C ../gf clean
	make -s -C ../rs clean
	rm -f test_gf
	rm -f test_rs
	rm -f *.o
