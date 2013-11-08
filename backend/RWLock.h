#ifndef RWLock_h
#define RWLock_h
#include <iostream>
#include <pthread.h>

class RWLock {
    public:
        RWLock();
        ~RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
        unsigned leyendo;
    	pthread_mutex_t turnstileMutex;
    	pthread_mutex_t roomEmptyMutex;
        pthread_mutex_t leyendoMutex;
};

#endif
