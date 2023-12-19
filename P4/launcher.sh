#!/bin/bash


for i in {1..50}
do
	if [ -e "E3_0/info$i.txt" ]; then
        rm "E3_0/info$i.txt"
    fi

  ./subscriber --ip  192.168.1.31 --port 1234 --topic motor  >> "E3_0/info$i.txt" &
  sleep 0.000001
done
wait


