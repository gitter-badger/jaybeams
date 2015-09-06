#!/bin/bash

# ... exit on the first error, print out what commands we will run ...
set -ev

# ... set the CPU power management to optimize for performance ...
sudo cpupower frequency-set -g performance

# ... make sure we reset the 
trap "sudo cpupower frequency-set -g ondemand" 0

# ... run different iterations of the benchmark ...
for test in double:aligned float:aligned double:unaligned float:unaligned; do
    echo ---
    ./jb/fftw/bm_time_delay_estimator --test-case=${test} --iterations=1000000
done


# ... print out compilation details ...
cat jb/testing/compile_info.cpp

# ... print out the available memory ...
free

# ... print out how many CPUs (or cores or whatevers) ...
egrep ^processor /proc/cpuinfo 

# ... print out the details of the first CPU ...
cat /proc/cpuinfo  | awk '{print $0; if ($0 == "") { exit; }}'

exit 0
