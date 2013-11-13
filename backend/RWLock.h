#ifndef RWLock_h
#define RWLock_h

#include <pthread.h>

class RWLock {
    public:
        RWLock();
        ~RWLock();
        void rlock();
        void runlock();
        void wlock();
        void wunlock();

    private:
        pthread_mutex_t turnstile;
        pthread_mutex_t room_empty;
        pthread_mutex_t lightswitch;
        unsigned counter;

        void lock_lightswitch();
        void unlock_lightswitch();
};

#endif
