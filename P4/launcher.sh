#!/bin/bash

for i in {1..10}
do
	if [ -e "info$i.txt" ]; then
        rm "info$i.txt"
    fi

  ./subscriber --ip 10.1.151.117 --port 1234 --topic motor  >> "info$i.txt" &
done
wait


