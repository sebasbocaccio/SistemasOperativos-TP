#ifndef LISTA_ATOMICA_HPP
#define LISTA_ATOMICA_HPP

#include<pthread.h>
#include <atomic>

template<typename T>
class ListaAtomica {
 private:
    struct Nodo {
        Nodo(const T &val) : _valor(val), _siguiente(nullptr) {}

        T _valor;
        Nodo *_siguiente;
    };
    pthread_mutex_t lock;
    std::atomic<Nodo *> _cabeza;

 public:
    ListaAtomica() : _cabeza(nullptr) {}

    ~ListaAtomica() {
        Nodo *n, *t;
        n = _cabeza.load();
        while (n) {
            t = n;
            n = n->_siguiente;
            delete t;
        }
    }

    void insertar(const T &valor) {
        // Completar (Ejercicio 1)

        Nodo* new_first = new Nodo(valor);
        pthread_mutex_lock(&lock);
        new_first->_siguiente = _cabeza.load();
        _cabeza.store(new_first);  
        pthread_mutex_unlock(&lock);
        // tempp <- cabeza
        //cabeza <- nuevo nodo
        //cabeza.siguiente <- temp
                
        
    }

    // Puede ser que cabeza no sea atomico??
    T &cabeza() const {
        return _cabeza.load()->_valor;
    }

    T &iesimo(unsigned int i) const {
        Nodo *n = _cabeza.load();
        for (unsigned int j = 0; j < i; j++) {
            n = n->_siguiente;
        }
        return n->_valor;
    }

    unsigned int longitud() const {
        Nodo *n = _cabeza.load();
        unsigned int cant = 0;
        while (n != nullptr) {
            cant++;
            n = n->_siguiente;
        }
        return cant;
    }


    class Iterador {
     private:
        ListaAtomica *_lista;

        typename ListaAtomica::Nodo *_nodo_sig;

        Iterador(ListaAtomica<T> *lista, typename ListaAtomica<T>::Nodo *sig)
            : _lista(lista), _nodo_sig(sig) {}

        friend typename ListaAtomica<T>::Iterador ListaAtomica<T>::crearIt();

     public:
        Iterador() : _lista(nullptr), _nodo_sig(nullptr) {}

        Iterador &operator=(const typename ListaAtomica::Iterador &otro) {
            _lista = otro._lista;
            _nodo_sig = otro._nodo_sig;
            return *this;
        }

        bool haySiguiente() const {
            return _nodo_sig != nullptr;
        }

        T &siguiente() {
            return _nodo_sig->_valor;
        }

        void avanzar() {
            _nodo_sig = _nodo_sig->_siguiente;
        }

        bool operator==(const typename ListaAtomica::Iterador &otro) const {
            return _lista->_cabeza.load() == otro._lista->_cabeza.load()
                && _nodo_sig == otro._nodo_sig;
        }
    };

    Iterador crearIt() {
        return Iterador(this, _cabeza);
    }
};

#endif /* LISTA_ATOMICA_HPP */
