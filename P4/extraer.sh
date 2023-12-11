#!/bin/bash

# Inicializa la variable i
i=1

# Itera sobre todos los archivos que cumplen con el patrón
for archivo in info*.txt; do
    # Extrae la columna de latencia utilizando cut y muestra el resultado
    
    if [ -e "latencia$i" ]; then
        rm "latencia$i"
    fi
    cut -d' ' -f 17 "$archivo" | cut -d: -f 1 >> "latencia$i"
    i=$((i + 1))
    
done
