#!/bin/bash
# created by manzumbado
# Benchmark runner

repeats=11
output_file='Benchmark_results.txt'
program='./mycc'
image_file='1.png'

run_tests() {
    local command_to_run+="$program $image_file"
    local time_array=()
    # --------------------------------------------------------------------------
    # Benchmark loop
    # --------------------------------------------------------------------------
    echo "Benchmarking  $command_to_run " ;

    # Indicate the command we just run in the csv file
    echo '======' $command_to_run '======' >> $output_file;

    # Run the given command [repeats] times
    for (( i = 1; i <= $repeats ; i++ ))
    do
        # percentage completion
        p=$(( $i * 100 / $repeats))
        # indicator of progress
        l=$(seq -s "+" $i | sed 's/[0-9]//g')

        # runs time function for the called script, output in a comma seperated
        # format output file specified with -o command and -a specifies append
        time_array[$i]=$({ /usr/bin/time -f "%e" $command_to_run; } 2>&1 )

        echo -ne ${l}' ('${p}'%) \r'
	echo "$i"
    done;
    echo 'Times = ' ${time_array[*]} #>> $output_file
    local sum_time=$( IFS="+"; bc <<< "${time_array[*]}" )
    echo 'Average time= '>> $output_file
    echo "$sum_time / $repeats" | bc -l >> $output_file
    echo -ne '\n'

    # Convenience seperator for file
    echo '--------------------------' >> $output_file
}


run_tests
