CFLAGS = -std=c99 -O3

ifneq ($(DEBUG_GF),)
  DEFS += -DDEBUG
endif

all: gf.o

%.o: %.c %.h ../ecc_cfg.h Makefile
	$(CC) -o $@ -c $(DEFS) $(CFLAGS) -I.. $<

clean:
	rm -f *.o
