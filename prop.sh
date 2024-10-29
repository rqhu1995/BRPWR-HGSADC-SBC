#!/bin/bash

cd build || exit

# seq 0 to 2.0 in 0.1 increments
for rpm in 1 2; do
    for tb in 7200 10800 14400 18000; do
        for i in $(seq 0.10 0.05 2.0); do
            ./main -ns 30 -bprop "$i" -ntrk 2 -nrpm "$rpm" -tb "$tb" -i 1001
        done
    done
done

# rpm: 3, tb: 14400, p: 0.10, count: 19
# rpm: 3, tb: 14400, p: 0.15, count: 18
# rpm: 3, tb: 14400, p: 0.20, count: 18
# rpm: 3, tb: 14400, p: 0.25, count: 16
# rpm: 3, tb: 14400, p: 0.30, count: 15
# rpm: 3, tb: 14400, p: 0.35, count: 15
# rpm: 3, tb: 14400, p: 0.40, count: 13
# rpm: 3, tb: 14400, p: 0.45, count: 12
# rpm: 3, tb: 14400, p: 0.50, count: 10
# rpm: 3, tb: 14400, p: 0.55, count: 14
# rpm: 3, tb: 14400, p: 0.60, count: 15
# rpm: 3, tb: 14400, p: 0.65, count: 16
# rpm: 3, tb: 14400, p: 0.70, count: 17
# rpm: 3, tb: 14400, p: 0.75, count: 17
# rpm: 3, tb: 14400, p: 0.80, count: 17
# rpm: 3, tb: 14400, p: 0.85, count: 17
# rpm: 3, tb: 14400, p: 0.90, count: 18
# rpm: 3, tb: 14400, p: 0.95, count: 18
# rpm: 3, tb: 14400, p: 1.00, count: 19
# rpm: 3, tb: 14400, p: 1.05, count: 19
# rpm: 3, tb: 14400, p: 1.10, count: 19
# rpm: 3, tb: 14400, p: 1.15, count: 19
# rpm: 3, tb: 14400, p: 1.20, count: 19
# rpm: 3, tb: 14400, p: 1.25, count: 19

# run rpm=3, tb=14400, p=xx 20-count times each

# # Define the parameters and their counts
# declare -A already_run_counts=(
#     ["0.10"]=19
#     ["0.15"]=18
#     ["0.20"]=18
#     ["0.25"]=16
#     ["0.30"]=15
#     ["0.35"]=15
#     ["0.40"]=13
#     ["0.45"]=12
#     ["0.50"]=10
#     ["0.55"]=14
#     ["0.60"]=15
#     ["0.65"]=16
#     ["0.70"]=17
#     ["0.75"]=17
#     ["0.80"]=17
#     ["0.85"]=17
#     ["0.90"]=18
#     ["0.95"]=18
#     ["1.00"]=19
#     ["1.05"]=19
#     ["1.10"]=19
#     ["1.15"]=19
#     ["1.20"]=19
#     ["1.25"]=19
# )

# rpm=3
# tb=14400

# # Loop through the parameters and run the command (20-count) times
# for p in "${!already_run_counts[@]}"; do
#     already_run_count=${already_run_counts[$p]}
#     # run the command (20-already_run_count) times, so that the total count is 20
#     for i in $(seq 1 $((20 - already_run_count))); do
#         ./main -ns 120 -bprop "$p" -ntrk 1 -nrpm "$rpm" -tb "$tb" -i 1
#     done
# done
