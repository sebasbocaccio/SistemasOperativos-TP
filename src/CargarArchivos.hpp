#ifndef HMC_ARCHIVOS_HPP
#define HMC_ARCHIVOS_HPP

#include <ctype.h>
#include <vector>
#include <string>
#include <pthread.h>

#include "HashMapConcurrente.hpp"

int cargarArchivo(
    HashMapConcurrente &hashMap,
    std::string filePath
);

void cargarMultiplesArchivos(
    HashMapConcurrente &hashMap,
    unsigned int cantThreads,
    std::vector<std::string> filePaths
);

typedef struct s_cargar_args {
   std::vector<std::string> filePaths; 
   std::atomic<int> &file_index;
   HashMapConcurrente &hm;
   
} cargar_args_t;

#endif /* HMC_ARCHIVOS_HPP */
