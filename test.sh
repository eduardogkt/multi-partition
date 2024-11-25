#!/bin/bash

input_size=$1
p_size=$2
threads=$3

for i in {1..10}; do
    ./partition $input_size $p_size $threads | grep CORRETO -c
done