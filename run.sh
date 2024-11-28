#!/bin/bash

# verifica se os argumentos <input_size> e <p_size> foram fornecidos
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "uso: ./run.sh <input_size> <p_size>"
    exit 1
fi

# compila
make

# cria diretório de resultados caso não exista
mkdir -p results

input_size=$1
p_size=$2

# inicializa arrays para armazenar os resultados
declare -A tempos
declare -A vazoes

result_file=results/results_${p_size}_${input_size}.txt

echo "Executando 5x com Input $input_size elementos, P $p_size elementos"

# roda 5 testes para cada quantidade de threads
for run in {1..5}; do
    echo -n "Medição $run: "

    for threads in {1..8}; do
        printf "$threads "

        output=$(./partition $input_size $p_size $threads)

        tempo=$(echo "$output" | grep -E "Tempo" | cut -d\  -f 2)
        vazao=$(echo "$output" | grep -E "Vazao" | cut -d\  -f 2)

        tempos[$run,$threads]=$tempo
        vazoes[$run,$threads]=$vazao
    done
    echo
done

# gera o arquivo com resultados
{
    echo "Tempo (s) input[$input_size] p[$p_size]"
    echo "num_threads, 1, 2, 3, 4, 5, 6, 7, 8"

    for run in {1..5}; do
        echo -n "run $run"
        for threads in {1..8}; do
            echo -n ", ${tempos[$run,$threads]}"
        done
        echo
    done

    echo

    echo "Vazao (MEPS) input[$input_size] p[$p_size]"
    echo "num_threads, 1, 2, 3, 4, 5, 6, 7, 8"

    for run in {1..5}; do
        echo -n "run $run"
        for threads in {1..8}; do
            echo -n ", ${vazoes[$run,$threads]}"
        done
        echo
    done
} > "$result_file"

echo "Resultados armazenados em $result_file"
