#ifndef HMC_HPP
#define HMC_HPP

#include <atomic>
#include <string>
#include <vector>

#include "ListaAtomica.hpp"

typedef std::pair<std::string, unsigned int> hashMapPair;

class HashMapConcurrente {
public:
   static const unsigned int cantLetras = 26;

   HashMapConcurrente();

   void incrementar(std::string clave);
   std::vector<std::string> claves();
   unsigned int valor(std::string clave);

   hashMapPair maximo();
   hashMapPair maximoParalelo(unsigned int cantThreads);

private:
   ListaAtomica<hashMapPair> *tabla[HashMapConcurrente::cantLetras];

   pthread_mutex_t lock[HashMapConcurrente::cantLetras];

   uint readers, writers;
   pthread_mutex_t mutex_readers, mutex_writers, room_empty, turnstile;

   pthread_mutex_t turnstiles_filas[HashMapConcurrente::cantLetras];
   pthread_mutex_t mutexes_filas[HashMapConcurrente::cantLetras];
   uint readers_filas[HashMapConcurrente::cantLetras];

   static unsigned int hashIndex(std::string clave);

   void* maximoThreads(void* args);

   typedef struct s_max_args {
      std::atomic<int> &table_index;
      hashMapPair &max;
      pthread_mutex_t &mutex_max;
   } max_args_t;

};

#endif  /* HMC_HPP */
