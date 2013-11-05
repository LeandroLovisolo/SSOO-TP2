#include "RWLock.h"

RWLock::RWLock() {
	//Inicializo el mutex
	pthread_mutex_init(&mutex, NULL);
}

void RWLock::rlock() {

}

void RWLock::wlock() {

}

void RWLock::runlock() {

}

void RWLock::wunlock() {

}

RWLock::~RWLock() {
pthread_mutex_destroy(&mutex)