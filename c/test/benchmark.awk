{
	if (NR % 2 == 0) {
		print $0, "x", $NF / t
	} else {
		t=$NF
		print
	}
}
