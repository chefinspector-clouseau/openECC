CFLAGS = -std=c99 -O3

DEFS += \
    -DCONFIG_REED_SOLOMON_ENC16 \
    -DCONFIG_REED_SOLOMON_DEC16

ifneq ($(DEBUG_LINUX),)
  DEFS += -DDEBUG
endif

all: rs-linux.o

%.o: %.c ../ecc_cfg.h ../gf/gf.h Makefile
	$(CC) -o $@ -c $(CFLAGS) $(DEFS) -I.. $<

#rs-linux.o: decode_rs.c encode_rs.c reed_solomon.c wrapper.c ../ecc_cfg.h ../gf/gf.h Makefile
#	$(CC) -o $@ -c $(DEFS) $(CFLAGS) -I.. reed_solomon.c wrapper.c

clean:
	rm -f *.o
