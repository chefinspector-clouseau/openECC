CFLAGS = -std=c99 -O3

all: rs.o

%.o: %.c %.h ../ecc_cfg.h ../gf/gf.h Makefile
	$(CC) -o $@ -c $(DEFS) $(CFLAGS) -I.. $<

clean:
	rm -f *.o
