#!/bin/bash

# Loop from 1 to 10
for i in {1..10}
do
  # Calculate start and end values for the current iteration
  start=$(( (i - 1) * 10 + 1 + 299 ))
  end=$(( i * 10 + 299))

  # Create a screen session with a name run-10-opt-i
  session_name="run-15-opt-$i"
  screen -S "$session_name" -dm bash -c "./run.sh 15 $start $end; exec bash"

  echo "Started screen session $session_name with ./run.sh 15 $start $end"

done
