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
    	pthread_mutex_t mutex;

};

#endif
