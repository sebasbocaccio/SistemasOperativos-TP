Claves()
=======

mutex_claves.wait()
    readers += 1
    if (readers == 1) :
        turnstile.wait()
        roomEmpty.wait()
mutex_claves.signal()

// Critical Section

mutex_claves.wait()
    readers -= 1
    if (readers == 0) :
        turnstile.signal()
        roomEmpty.signal()
mutex_claves.signal()


PRE = {clave € Claves()}
Valor(i)
=======

mutexes_filas[i].wait()
    readers_filas[i] += 1
    if (readers_filas[i]== 1) :
        turnstile_filas[i].wait()
        lock_filas[i].wait()
mutexes_filas[i].signal()

// Critical Section
    leer(tabla[i])

mutexes_filas[i].wait()
    readers_filas[i] -= 1
    if (readers_filas[i] == 0) :
        turnstile_filas[i].signal()
        lock_filas[i].signal()
mutexes_filas[i].signal()
