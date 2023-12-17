#!/bin/bash


for i in {1..50}
do
	if [ -e "info$i.txt" ]; then
        rm "info$i.txt"
    fi

  ./subscriber --ip 192.168.1.178 --port 1234 --topic motor  >> "E1_0/info$i.txt" &
  sleep 0.5
done
wait


