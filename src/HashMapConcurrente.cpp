#ifndef CHM_CPP
#define CHM_CPP

#include <iostream>
#include <fstream>
#include <pthread.h>

#include "HashMapConcurrente.hpp"

HashMapConcurrente::HashMapConcurrente() {
    for (unsigned int i = 0; i < HashMapConcurrente::cantLetras; i++) {
        tabla[i] = new ListaAtomica<hashMapPair>();
    }
}

unsigned int HashMapConcurrente::hashIndex(std::string clave) {
    return (unsigned int)(clave[0] - 'a');
}

void HashMapConcurrente::incrementar(std::string clave) {
    ListaAtomica<hashMapPair> *lista  = tabla[hashIndex(clave)];
    ListaAtomica<hashMapPair>::Iterador it = lista->crearIt();
    
    pthread_mutex_lock(&lock[hashIndex(clave)]);
    while(it.haySiguiente() && it.siguiente().first != clave) {
        it.avanzar();
    }

    if(!it.haySiguiente()) {
        hashMapPair nuestraTupla = hashMapPair(clave, 1);
        lista->insertar(nuestraTupla);
    } else {
        it.siguiente().second++;
    }
    
    pthread_mutex_unlock(&lock[hashIndex(clave)]);
}

std::vector<std::string> HashMapConcurrente::claves() {
    std::vector<std::string> res;

    for(unsigned int i = 0; i < HashMapConcurrente::cantLetras ; i++) {
        ListaAtomica<hashMapPair>::Iterador it = tabla[i]->crearIt();
        
        while(it.haySiguiente()){
            res.push_back(it.siguiente().first);
            it.avanzar();
        }
    }

    return res;
}

unsigned int HashMapConcurrente::valor(std::string clave) {
    ListaAtomica<hashMapPair>::Iterador it = tabla[hashIndex(clave)]->crearIt();
    
    while(it.haySiguiente() && it.siguiente().first != clave) {
        it.avanzar();
    }

    if(!it.haySiguiente()) return 0;
    else return it.siguiente().second;
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