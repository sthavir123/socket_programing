#!/bin/bash
for i in 1 2 3 4
do
    g++ phase1.cpp -o client$i
done

#for i in 1 2 3 
#do
#    ./client$i client$i-config.txt files/client$i/ &
#done
#./client4 client4-config.txt files/client4/ 
