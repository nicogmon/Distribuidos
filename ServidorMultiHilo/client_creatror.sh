#!/bin/bash

for i in `seq 1 200`; do
	WAIT=`printf '0.%06d\n' $RANDOM`;
	
	(sleep $WAIT; echo "Lanzando cliente $i ..."; ./client $i 10.1.146.127 6000) &
done
