#!/bin/bash


for i in {1..50}
do
	if [ -e "E1_0/info$i.txt" ]; then
        rm "E1_0/info$i.txt"
    fi

  ./subscriber --ip  212.128.254.74 --port 1234 --topic motor  >> "E1_0/info$i.txt" &
  sleep 0.000001
done
wait


