#!/bin/bash

# Encuentra todos los archivos que coinciden con el patrón "E2_0/info*txt"
files=(E3_0/info*txt)


for file in "${files[@]}"; do
    echo $file
    identifier=$(echo "$file" | grep -oP 'info(\d+)' | grep -oP '\d+')
    echo $identifier
    if [ -e "E1_0/latencia$identifier" ]; then
        rm "E1_0/latencia$identifier"
    fi
    
    # Obtiene solo los valores de latencia y los imprime
    grep -oP 'Latencia: \K[\d.]+' "$file" >> "E3_0/latencia$identifier"
    
done