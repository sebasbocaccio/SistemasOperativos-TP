Incrementar(i)
=======

turnstile.wait()
turnstile.signal()

mutex_incrementar.wait()
    writers += 1
    if (writers == 1) :
        roomEmpty.wait()
mutex_incrementar.signal()

turnstile_filas[i].wait()
turnstile_filas[i].signal()

lock_filas[i].wait()

// Critical Section

lock_filas[i].signal()

mutex_writers.wait()
    writers -= 1
    if (writers == 0) :
        roomEmpty.signal()
mutex_writers.signal()