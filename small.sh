#!/bin/bash

cd build || exit

n_runs=20

for ns in 6 10 15; do
    for i in {1..5}; do
        for ((j = 1; j <= n_runs; j++)); do
            ./main -ns "$ns" -i "$i" -edu 40 -tb 7200
        done
    done
done
