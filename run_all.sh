#/bin/bash

# programa para rodar todos os testes de uma vez

# se o programa não for espeficicado, roda ambos a e b
if [ -z "$1" ]; then
    ./run_all.sh a
    ./run_all.sh b
    exit 0
fi

echo "Testando todos inputs para parte $1"

for input in {1000000,2000000,4000000,8000000,16000000}; do
    read -p "Testar com input $input? (y/n) " resposta
    
    if [[ "$resposta" != "y" ]]; then
        echo "Pulando o valor $input."
        continue  # pula para a próxima iteração
    fi

    ./run_tests.sh $1 $input
done