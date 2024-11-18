#!/bin/bash

# verifica se os argumentos <exe> e <input_size> foram fornecidos
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "uso: ./run_tests.sh <a/b> <input_size>"
    exit 1
fi

# define o executável com base no argumento <exe>
if [ "$1" == "a" ]; then
    exe="bsearch_a"
elif [ "$1" == "b" ]; then
    exe="bsearch_b"
else
    echo "Erro: <exe> deve ser 'a' ou 'b'"
    exit 1
fi

# compila
make

# cria diretorio de resultados caso nao exista
mkdir -p results

input_size=$2
result_vazao=results/result_${1}_vazao_${input_size}.txt
result_tempo=results/result_${1}_tempo_${input_size}.txt

echo "Executando 10x com input de $input_size elementos"

echo "Vazao (MOPS) $input_size" > $result_vazao
echo "Tempo (s) $input_size" > $result_tempo

echo "num_threads, 1, 2, 3, 4, 5, 6, 7, 8" >> $result_vazao
echo "num_threads, 1, 2, 3, 4, 5, 6, 7, 8" >> $result_tempo

# roda 10 testes para cada quantidade de threads
for run in {1..10}; do
    printf "Medição $run: "

    vazao_line="run $run"
    tempo_line="run $run"

    for threads in {1..8}; do
        printf "$threads "

        output=$(./$exe $input_size $threads)

        vazao=$( echo "$output" | grep -E "Vazao" | cut -d\  -f 2 )
        vazao_line+=", ${vazao}"

        tempo=$( echo "$output" | grep -E "Tempo" | cut -d\  -f 2 )
        tempo_line+=", ${tempo}"
    done

    echo
    echo "$vazao_line" >> $result_vazao
    echo "$tempo_line" >> $result_tempo
done