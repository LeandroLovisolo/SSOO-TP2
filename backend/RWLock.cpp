#include "RWLock.h"

//Esta solución puede hacer que los writes tarden más que los reads (Es un problema?)

RWLock::RWLock() {
	//Inicializo el mutex
	pthread_mutex_init(&turnstileMutex, NULL);
	pthread_mutex_init(&roomEmptyMutex, NULL);
}

void RWLock::rlock() {
	pthread_mutex_lock(&turnstileMutex);
	pthread_mutex_unlock(&turnstileMutex);

	pthread_mutex_lock(&roomEmptyMutex);
}

void RWLock::wlock() {
	//Bloqueo el molinete y room empty
	pthread_mutex_lock(&turnstileMutex);
	pthread_mutex_lock(&roomEmptyMutex);
}

void RWLock::runlock() {
	pthread_mutex_unlock(&roomEmptyMutex);
}

void RWLock::wunlock() {
	pthread_mutex_unlock(&turnstileMutex);
	pthread_mutex_unlock(&roomEmptyMutex);
}

RWLock::~RWLock() {
	pthread_mutex_destroy(&turnstileMutex);
	pthread_mutex_destroy(&roomEmptyMutex);
}