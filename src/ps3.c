turnstile = semaphore(1);
roomEmpty() = semaphore(1);
readers = 0;
writers = 0;
mutex_readers = mutex();
mutex_writers = mutex()