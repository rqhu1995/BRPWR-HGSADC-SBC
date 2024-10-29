#!/bin/bash
cd build || exit
tb=$1
# Define the maximum number of parallel jobs
max_parallel_jobs=4 # 16 threads / 4 threads per instance

# Define the station and instance pairs
declare -A station_instances
station_instances=(
  [60]="1"
  [90]="1"
  [120]="1"
  [200]="1"
  [300]="1"
  [400]="1"
  [500]="1"
)

# Define the number of runs
num_runs=20

# Iterate over each station and its corresponding instances
for n_station in "${!station_instances[@]}"; do
  # Split the instance string into an array
  IFS=' ' read -r -a inst_list <<<"${station_instances[$n_station]}"

  for inst in "${inst_list[@]}"; do
    ntrk_values=(1 2)
    nrpm_values=(1 2)

    for ntrk in "${ntrk_values[@]}"; do
      for nrpm in "${nrpm_values[@]}"; do
        for ((i = 1; i <= num_runs; i++)); do
          # Generate a unique session name for each run
          session_name="ns_${n_station}_inst_${inst}_ntrk_${ntrk}_nrpm_${nrpm}_run_${i}"

          # Start a new screen session and run the command
          screen -dmS "$session_name" bash -c "
                        ./main -ns '$n_station' -i '$inst' -edu 20 -ntrk '$ntrk' -nrpm '$nrpm' -noimp 5000 -tb '$tb';
                        exit
                    "

          # Monitor the number of active screen sessions
          while [ "$(screen -ls | grep -c 'ns_')" -ge "$max_parallel_jobs" ]; do
            sleep 10 # Check every 10 seconds if the sessions have finished
          done
        done
      done
    done
  done
done

# Final wait to ensure all sessions complete before the script exits
wait
