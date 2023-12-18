#!/bin/bash


for i in {1..901}
do
	if [ -e "E1_0/info$i.txt" ]; then
        rm "E1_0/info$i.txt"
    fi

  ./subscriber --ip  192.168.145.194 --port 1234 --topic motor  >> "E1_0/info$i.txt" &
  sleep 0.000001
done
wait


