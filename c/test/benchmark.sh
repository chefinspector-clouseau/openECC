#!/bin/bash

time=/usr/bin/time

SEED=1
MAKECMD=

benchmark() {
	codec=$1	; shift
	bits=$1		; shift
	n=$1		; shift
	n_k=$1		; shift
	nErrs=$1	; shift
	nEnc=$1		; shift
	nDec=$1		; shift
	makecmd="make CODEC=$codec"
	# only recompile if required:
	if [[ $makecmd != $MAKECMD ]]; then
		make clean > /dev/null
		eval $makecmd > /dev/null
		MAKECMD=$makecmd
	fi
	status=
	t=$($time -f '%U' ./test_rs $bits $n $n_k 1 $nErrs $SEED $nEnc $nDec 2>&1) || \
		{ t=99.99; status=FAILED; }
	if (( $nEnc > 1 )); then
		mode=enc
	elif (( $nDec > 1 )); then
		mode=dec
	fi
	printf "%s %2d %5d %4d %4d %10s %10.2f %s\n" \
		$mode $bits $n $n_k $nErrs $codec $t $status
}

# nEnc and nDec are chosen so that overall time is roughly 1s
# (on my old Mac)

#                  codec bits     n  n_k nErrs    nEnc    nDec
# ------------------------------------------------------------
for codec in mine linux; do    

	# GF(2^8):	       
	benchmark $codec    8   255    2     0  400000       1
	benchmark $codec    8   255    2     0       1  600000
	benchmark $codec    8   255    2     1       1  400000
	benchmark $codec    8   100    2     0 1000000       1
	benchmark $codec    8   100    2     0       1 1000000
	benchmark $codec    8   100    2     1       1 1000000
	benchmark $codec    8   255   16     0  100000       1
	benchmark $codec    8   255   16     0       1  100000
	benchmark $codec    8   255   16     8       1  100000
	benchmark $codec    8   255   32     0   50000       1
	benchmark $codec    8   255   32     0       1   50000
	benchmark $codec    8   255   32    16       1   50000
	benchmark $codec    8   255   64     0   40000       1
	benchmark $codec    8   255   64     0       1   20000
	benchmark $codec    8   255   64    32       1   20000
	benchmark $codec    8   100   64     0  100000       1
	benchmark $codec    8   100   64     0       1  100000
	benchmark $codec    8   100   64    32       1   20000
	benchmark $codec    8   255  128     0   40000       1
	benchmark $codec    8   255  128     0       1   10000
	benchmark $codec    8   255  128    64       1   10000
	benchmark $codec    8   255  200     0   40000       1
	benchmark $codec    8   255  200     0       1   10000
	benchmark $codec    8   255  200   100       1   10000
			       
	# GF(2^12):	       
	benchmark $codec   12  4095    2     0   30000       1
	benchmark $codec   12  4095    2     0       1   20000
	benchmark $codec   12  4095    2     1       1   20000
	benchmark $codec   12   100    2     0  500000       1
	benchmark $codec   12   100    2     0       1 1000000
	benchmark $codec   12   100    2     1       1  100000
	benchmark $codec   12  4095   64     0    2000       1
	benchmark $codec   12  4095   64     0       1    2000
	benchmark $codec   12  4095   64    32       1    1000
	benchmark $codec   12   256   64     0   40000       1
	benchmark $codec   12   256   64     0       1   30000
	benchmark $codec   12   256   64    32       1   20000
	benchmark $codec   12  4095  256     0     500       1
	benchmark $codec   12  4095  256     0       1     500
	benchmark $codec   12  4095  256   128       1     500
	benchmark $codec   12  1024  256     0    2000       1
	benchmark $codec   12  1024  256     0       1    2000
	benchmark $codec   12  1024  256   128       1    2000
	benchmark $codec   12  4095 1024     0     500       1
	benchmark $codec   12  4095 1024     0       1     500
	benchmark $codec   12  4095 1024   512       1     500
	benchmark $codec   12  4095 2048     0     500       1
	benchmark $codec   12  4095 2048     0       1     500
	benchmark $codec   12  4095 2048  1024       1     500
			       
	# GF(2^16):	       
	benchmark $codec   16 65535    2     0    1000       1
	benchmark $codec   16 65535    2     0       1    1000
	benchmark $codec   16 65535    2     1       1    1000
	benchmark $codec   16 65535 1024     0     100       1
	benchmark $codec   16 65535 1024     0       1     100
	benchmark $codec   16 65535 1024   512       1     100
	benchmark $codec   16   256   64     0   30000       1
	benchmark $codec   16   256   64     0       1   20000
	benchmark $codec   16   256   64    32       1    4000
done | tee -a benchmark.log

echo "--------------------------------------------------------------------------------"
sort benchmark.log | uniq | awk -f benchmark.awk
