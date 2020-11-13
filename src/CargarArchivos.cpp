#ifndef CHM_CPP
#define CHM_CPP

#include <vector>
#include <iostream>
#include <fstream>
#include <pthread.h>

#include "CargarArchivos.hpp"

int cargarArchivo(
    HashMapConcurrente &hashMap,
    std::string filePath
) {
    std::fstream file;
    int cant = 0;
    std::string palabraActual;

    // Abro el archivo.
    file.open(filePath, file.in);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo '" << filePath << "'" << std::endl;
        return -1;
    }
    while (file >> palabraActual) {
        hashMap.incrementar(palabraActual);
        cant++;
    }
    // Cierro el archivo.
    if (!file.eof()) {
        std::cerr << "Error al leer el archivo" << std::endl;
        file.close();
        return -1;
    }
    file.close();
    return cant;
}


void* cargarArchivosThread(void* args){
    cargar_args_t c_arg = *((cargar_args_t*) args); // Aca cargamos los argumentos .. 

    unsigned int index;
    while((index= c_arg.file_index.fetch_add(1)) < c_arg.filePaths.size()){
        cargarArchivo(c_arg.hm,c_arg.filePaths[index]);
    } 
    return nullptr;

}

void cargarMultiplesArchivos(
    HashMapConcurrente &hashMap,
    unsigned int cantThreads,
    std::vector<std::string> filePaths
) {
    pthread_t tid[cantThreads];
    std::atomic<int> file_index(0);

    cargar_args_t args = {
        filePaths, 
        file_index,
        hashMap,
    };

        // Inicializamos los threads
    for (unsigned int i = 0; i < cantThreads; ++i) {
        pthread_create(tid + i, NULL, cargarArchivosThread, &args);
    }

    // Esperamos a que terminen todos
    for (unsigned int i = 0; i < cantThreads; ++i) {
        pthread_join(tid[i], NULL);
    }


}


#endif
