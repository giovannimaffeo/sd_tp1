#!/bin/bash

# Função que executa o código e registra o tempo médio de execução
run_code() {
    N=$1
    Np=$2
    Nc=$3

    total_time=0

    for ((i=1; i<=10; i++))
    do
        echo "Execução $i - (N=$N, Np=$Np, Nc=$Nc)"
        start=$(date +%s%N)
        ./producer_consumer $N $Np $Nc
        end=$(date +%s%N)
        runtime=$((end-start))

        # Adiciona o tempo de execução ao total
        total_time=$((total_time + runtime))
    done

    # Calcula o tempo médio
    average_time=$((total_time / 10))

    echo "Tempo médio: $average_time nanossegundos"
    echo ""
}

# Executa o código para diferentes combinações de N, Np e Nc
run_code 1 1 1
run_code 1 1 2
run_code 1 1 4
run_code 1 1 8
run_code 1 2 1
run_code 1 4 1
run_code 1 8 1

run_code 10 1 1
run_code 10 1 1
run_code 10 1 2
run_code 10 1 4
run_code 10 1 8
run_code 10 2 1
run_code 10 4 1
run_code 10 8 1

run_code 100 1 1
run_code 100 1 1
run_code 100 1 2
run_code 100 1 4
run_code 100 1 8
run_code 100 2 1
run_code 100 4 1
run_code 100 8 1

run_code 1000 1 1
run_code 1000 1 1
run_code 1000 1 2
run_code 1000 1 4
run_code 1000 1 8
run_code 1000 2 1
run_code 1000 4 1
run_code 1000 8 1