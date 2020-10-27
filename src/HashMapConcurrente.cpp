#ifndef CHM_CPP
#define CHM_CPP

#include <iostream>
#include <fstream>
#include <pthread.h>

#include "HashMapConcurrente.hpp"

HashMapConcurrente::HashMapConcurrente() {
    for (unsigned int i = 0; i < HashMapConcurrente::cantLetras; i++) {
        tabla[i] = new ListaAtomica<hashMapPair>();
        
        pthread_mutex_init(lock + i, NULL);
        pthread_mutex_init(turnstiles_filas + i, NULL);
        pthread_mutex_init(mutexes_filas + i, NULL);
        
        readers_filas[i] = 0;
    }

    pthread_mutex_init(&mutex_readers, NULL);
    pthread_mutex_init(&mutex_writers, NULL);
    pthread_mutex_init(&room_empty, NULL);
    pthread_mutex_init(&turnstile, NULL);

    readers = 0;
    writers = 0;
}

unsigned int HashMapConcurrente::hashIndex(std::string clave) {
    return (unsigned int)(clave[0] - 'a');
}

void HashMapConcurrente::incrementar(std::string clave) {
    pthread_mutex_lock(&mutex_writers);
        writers += 1;
        if (writers == 1) {
            pthread_mutex_lock(&turnstile);
            pthread_mutex_lock(&room_empty);
        }
    pthread_mutex_unlock(&mutex_writers);
    
    /// Inicio sección crítica

    ListaAtomica<hashMapPair>::Iterador it = tabla[hashIndex(clave)]->crearIt();
    
    pthread_mutex_lock(turnstiles_filas + hashIndex(clave));
    pthread_mutex_lock(lock + hashIndex(clave));

        while(it.haySiguiente() && it.siguiente().first != clave) {
            it.avanzar();
        }

        if(!it.haySiguiente()) {
            hashMapPair nuestraTupla = hashMapPair(clave, 1);
            tabla[hashIndex(clave)]->insertar(nuestraTupla);
        } else {
            it.siguiente().second++;
        }
    
    pthread_mutex_unlock(turnstiles_filas + hashIndex(clave));
    pthread_mutex_unlock(lock + hashIndex(clave));

    /// Fin sección crítica

    pthread_mutex_lock(&mutex_writers);
        writers -= 1;
        if (writers == 0) {
            pthread_mutex_unlock(&turnstile);
            pthread_mutex_unlock(&room_empty);
        }
    pthread_mutex_unlock(&mutex_writers);
}

std::vector<std::string> HashMapConcurrente::claves() {
    std::vector<std::string> res;

    pthread_mutex_lock(&turnstile);
    pthread_mutex_unlock(&turnstile);

    pthread_mutex_lock(&mutex_readers);
        readers += 1;
        if (readers == 1) {
            pthread_mutex_lock(&room_empty);
        }
    pthread_mutex_unlock(&mutex_readers);

    /// Inicio sección crítica

    for(unsigned int i = 0; i < HashMapConcurrente::cantLetras ; i++) {
        ListaAtomica<hashMapPair>::Iterador it = tabla[i]->crearIt();
        
        while(it.haySiguiente()){
            res.push_back(it.siguiente().first);
            it.avanzar();
        }
    }

    /// Fin sección crítica

    pthread_mutex_lock(&mutex_readers);
        readers -= 1;
        if (readers == 0) {
            pthread_mutex_unlock(&room_empty);
        }
    pthread_mutex_unlock(&mutex_readers);

    return res;
}

unsigned int HashMapConcurrente::valor(std::string clave) {
    pthread_mutex_lock(turnstiles_filas + hashIndex(clave));
    pthread_mutex_unlock(turnstiles_filas + hashIndex(clave));

    pthread_mutex_lock(mutexes_filas + hashIndex(clave));
        readers_filas[hashIndex(clave)] += 1;
        if (readers_filas[hashIndex(clave)] == 1) {
            pthread_mutex_lock(lock + hashIndex(clave));
        }
    pthread_mutex_unlock(mutexes_filas + hashIndex(clave));

    /// Inicio sección crítica

    ListaAtomica<hashMapPair>::Iterador it = tabla[hashIndex(clave)]->crearIt();
    
    while(it.haySiguiente() && it.siguiente().first != clave) {
        it.avanzar();
    }

    if(!it.haySiguiente()) {
        return 0;
    } else {
        return it.siguiente().second;
    }

    /// Fin sección crítica

    pthread_mutex_lock(mutexes_filas + hashIndex(clave));
        readers_filas[hashIndex(clave)] -= 1;
        if (readers_filas[hashIndex(clave)] == 0) {
            pthread_mutex_unlock(lock + hashIndex(clave));
        }
    pthread_mutex_unlock(mutexes_filas + hashIndex(clave));
}

hashMapPair HashMapConcurrente::maximo() {
    hashMapPair *max = new hashMapPair();
    max->second = 0;

    for (unsigned int index = 0; index < HashMapConcurrente::cantLetras; index++) {
        for (
            auto it = tabla[index]->crearIt();
            it.haySiguiente();
            it.avanzar()
        ) {
            if (it.siguiente().second > max->second) {
                max->first = it.siguiente().first;
                max->second = it.siguiente().second;
            }
        }
    }

    return *max;
}

hashMapPair HashMapConcurrente::maximoParalelo(unsigned int cantThreads) {
    // Completar (Ejercicio 3)
}

#endif