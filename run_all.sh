#/bin/bash

# programa para rodar todos os testes de uma vez

echo "Testando para diferentes tamanhos de P"

for p_size in {1000,100000}; do
    read -p "Testar com P $p_size? (y/n) " resposta
    
    if [[ "$resposta" != "y" ]]; then
        echo "Pulando o valor $p_size."
        continue  # pula para a próxima iteração
    fi

    ./run.sh 16000000 $p_size
done