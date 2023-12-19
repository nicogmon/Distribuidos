import csv
import statistics
import os

def process_files(file_paths):
    data = []

    # Procesa cada archivo
    for file_path in file_paths:
        with open(file_path, 'r') as file:
            values = [float(line.strip().split()[-1]) for line in file if line.strip().split()]
            if values:
                min_val = min(values)
                max_val = max(values)
                avg_val = statistics.mean(values)
                std_val = statistics.stdev(values)

                data.append({
                    'Min': min_val,
                    'Max': max_val,
                    'Avg': avg_val,
                    'Std': std_val
                })

    return data

def calculate_aggregates(data):
    min_of_mins = min(row['Min'] for row in data)
    max_of_maxs = max(row['Max'] for row in data)
    avg_of_avgs = statistics.mean(row['Avg'] for row in data)
    std_of_stds = statistics.stdev(row['Std'] for row in data)

    return {
        'Min': min_of_mins,
        'Max': max_of_maxs,
        'Avg': avg_of_avgs,
        'Std': std_of_stds
    }

def write_aggregates_csv(output_file, aggregates):
    with open(output_file, 'a', newline='') as csvfile:
        fieldnames = ['Mode', 'N', 'Min', 'Max', 'Avg', 'Std']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

        

        # Escribe la fila con los valores agregados
        writer.writerow({
            'Mode': 2,
            'N': 50,
            'Min': aggregates['Min'],
            'Max': aggregates['Max'],
            'Avg': aggregates['Avg'],
            'Std': aggregates['Std']
        })

if __name__ == "__main__":
    # Especifica la lista de archivos a procesar
#for j in range(1,4):
    #for k in range(3):
    file_paths = []
    for i in range(1,51):
        file_paths.append("E3_0/latencia"+str(i))
            # Procesa los archivos
    result_data = process_files(file_paths)

    # Calcula los agregados de los resultados
    aggregates = calculate_aggregates(result_data)

    # Especifica el nombre del archivo de salida para los agregados
    output_aggregates_file = "agregados.csv"

    # Escribe los resultados agregados en un archivo CSV con el formato específico
    write_aggregates_csv(output_aggregates_file, aggregates)

    print(f"Los resultados agregados han sido guardados en {output_aggregates_file}")
