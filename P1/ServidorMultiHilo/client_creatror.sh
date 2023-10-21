#!/bin/bash

for i in `seq 1 200`; do
	WAIT=`printf '0.%06d\n' $RANDOM`;
	
	(sleep $WAIT; echo "Lanzando cliente $i ..."; ./client $i 192.168.1.31 6000) &
done


