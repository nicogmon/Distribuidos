#!/bin/bash

# Encuentra todos los archivos que coinciden con el patrón "E2_0/info*txt"
folders=( $(find . -maxdepth 1 -type d -name 'E*_?' | sort) )

for folder in "${folders[@]}"; do
    echo "Carpeta: $folder"
    files=($folder/info*txt)


    for file in "${files[@]}"; do
        echo $file
        identifier=$(echo "$file" | grep -oP 'info(\d+)' | grep -oP '\d+')
        echo $identifier
        if [ -e "$folder/latencia$identifier" ]; then
            rm "$folder/latencia$identifier"
        fi

        
        # Obtiene solo los valores de latencia y los imprime
        grep -oP 'Latencia: \K[\d.]+' "$file" >> "$folder/latencia$identifier"
        
    done
done