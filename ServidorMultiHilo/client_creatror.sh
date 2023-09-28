#!/bin/bash

for i in `seq 1 200`; do
	WAIT=`printf '0.%06d\n' $RANDOM`;
	
	(sleep $WAIT; echo "Lanzando cliente $i ..."; ./client $i 212.128.254.45 8000) &
done
