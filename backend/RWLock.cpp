#include <RWLock.h>

#define wait(x)   pthread_mutex_lock(&x)
#define signal(x) pthread_mutex_unlock(&x)

// Lock readers-writers sin inanición, tomado de The Little Book of Semaphores,
// versión 2.1.5, por Allen B. Downey; página 79.

// Comentar la siguiente línea para obtener la implementación con inanición,
// para verificar que lo logra pasar los tests de inanición en tests.cpp.
#define SIN_INANICION

RWLock::RWLock() {
	pthread_mutex_init(&turnstile, NULL);
	pthread_mutex_init(&room_empty, NULL);
	pthread_mutex_init(&lightswitch, NULL);
	counter = 0;
}

RWLock::~RWLock() {
	pthread_mutex_destroy(&turnstile);
	pthread_mutex_destroy(&lightswitch);
	pthread_mutex_destroy(&room_empty);
}

void RWLock::rlock() {
	#ifdef SIN_INANICION
	wait(turnstile);
	signal(turnstile);
	#endif
	lock_lightswitch();
}

void RWLock::runlock() {
	unlock_lightswitch();
}

void RWLock::wlock() {
	#ifdef SIN_INANICION
	wait(turnstile);
	#endif
	wait(room_empty);
}

void RWLock::wunlock() {
	#ifdef SIN_INANICION
	signal(turnstile);
	#endif
	signal(room_empty);
}

void RWLock::lock_lightswitch() {
	wait(lightswitch);
	counter++;
	if(counter == 1) {
		wait(room_empty);
	}
	signal(lightswitch);
}

void RWLock::unlock_lightswitch() {
	wait(lightswitch);
	counter--;
	if(counter == 0) {
		signal(room_empty);
	}
	signal(lightswitch);
}