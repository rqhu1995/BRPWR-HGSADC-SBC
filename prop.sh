#!/bin/bash

cd build || exit

# seq 0 to 2.0 in 0.1 increments
for rpm in 1 2 3 4; do
    for tb in 7200 10800 14400 18000; do
        for i in $(seq 0.05 2.0 0.05); do
            ./main -ns 120 -bprop "$i" -ntrk 1 -nrpm "$rpm" -tb "$tb" -i 1
        done
    done
done
