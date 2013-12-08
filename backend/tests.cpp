#include <vector>
#include <RWLock.h>
#include <pthread.h>

#include "gtest/gtest.h"

using namespace std;

// Tiempo de espera en microsegundos del método RWLockTest::esperar().
#define TIEMPO_DE_ESPERA 10000

// Número de iteraciones del test de inanición de escritura.
#define NUM_TESTS_DE_INANICION 250

///////////////////////////////////////////////////////////////////////////////
// Fixture                                                                   //
///////////////////////////////////////////////////////////////////////////////

typedef enum { LECTURA, ESCRITURA } evento;

// Se crea una nueva instancia de esta clase para cada test.

class RWLockTest : public testing::Test {
public:
	RWLock lock;
	vector<evento> eventos;

protected:
	void esperar() {
		usleep(TIEMPO_DE_ESPERA);
	}

	// Convierte el vector de eventos a texto (para imprimir por consola.)
	string eventos_to_str() {
		string s = "eventos = [";
		for(size_t i = 0; i < eventos.size(); i++) {
			if(i > 0) s += ", ";
			switch(eventos[i]) {
				case LECTURA:   s += "LECTURA";   break;
				case ESCRITURA: s += "ESCRITURA"; break;
			}
		}
		s += "]";
		return s;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Threads                                                                   //
///////////////////////////////////////////////////////////////////////////////

// Puntos de entrada de los threads usados en los tests.

void* read(void* arg) {
	RWLockTest* test = (RWLockTest*) arg;
	test->lock.rlock();
	test->eventos.push_back(LECTURA);
	test->lock.runlock();
	return NULL;
}

void* write(void* arg) {
	RWLockTest* test = (RWLockTest*) arg;
	test->lock.wlock();
	test->eventos.push_back(ESCRITURA);
	test->lock.wunlock();
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// TEST: Pueden realizarse varias lecturas a la vez                          //
///////////////////////////////////////////////////////////////////////////////

TEST_F(RWLockTest, AdmiteVariasLecturasALaVez) {
	// Obtengo lock para más de una lectura.
	lock.rlock();
	lock.rlock();

	// Libero los locks para las lecturas anteriores.
	lock.runlock();
	lock.runlock();

	// Si el lock admite varios lectores a la vez, entonces el proceso no
	// debería bloquearse al pedir el segundo lock. Consideramos exitoso el
	// test si llega a ejecutarse la siguiente línea de código.
	EXPECT_TRUE(true);
}

///////////////////////////////////////////////////////////////////////////////
// TEST: Se realiza una sola escritura a la vez                              //
///////////////////////////////////////////////////////////////////////////////

TEST_F(RWLockTest, AdmiteUnaSolaEscrituraALaVez) {
	// Obtengo lock de escritura.
	lock.wlock();

	// Creo un thread que intenta escribir.
	pthread_t writer;
	pthread_create(&writer, NULL, write, this);

	// Le doy tiempo al thread.
	esperar();

	// Verifico que el thread no haya podido realizar la escritura,
	// pues habíamos tomado un lock de escritura.
	EXPECT_EQ(0, eventos.size());

	// Libero el lock de escritura y espero que termine el thread.
	lock.wunlock();
	pthread_join(writer, NULL);

	// Verifico que el thread ahora sí haya podido escribir.
	EXPECT_EQ(ESCRITURA, eventos[0]);	

	// Espero que termine el thread.
	pthread_join(writer, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// TEST: No se escribe mientras se lee                                       //
///////////////////////////////////////////////////////////////////////////////

TEST_F(RWLockTest, NoSeEscribeMientrasSeLee) {
	// Número de lecturas que se realizan antes de escribir
	int n = 3;

	// Obtengo locks de lectura.
	for(int i = 0; i < n; i++) lock.rlock();

	// Creo un thread que intenta escribir.
	pthread_t writer;
	pthread_create(&writer, NULL, write, this);

	for(int i = 0; i < n; i++) {
		// Le doy tiempo al thread para ejecutarse.
		esperar();

		// Verifico que el thread no haya podido realizar la escritura,
		// pues todavía se tiene tomado al menos un lock de lectura.
		EXPECT_EQ(0, eventos.size());

		// Libero el i-ésimo lock de lectura.
		lock.runlock();
	}

	// Luego de haber liberado el último lock de lectura, el thread de escritura
	// ahora debería desbloquearse. Espero que termine el thread.	
	pthread_join(writer, NULL);

	// Verifico que el thread ahora sí haya podido escribir.
	EXPECT_EQ(ESCRITURA, eventos[0]);
}

///////////////////////////////////////////////////////////////////////////////
// TEST: No se lee mientras se escribe                                       //
///////////////////////////////////////////////////////////////////////////////

TEST_F(RWLockTest, NoSeLeeMientrasSeEscribe) {
	// Obtengo lock de escritura.
	lock.wlock();

	// Creo un thread que intenta leer.
	pthread_t reader;
	pthread_create(&reader, NULL, read, this);

	// Le doy tiempo al thread.
	esperar();

	// Verifico que el thread no haya podido realizar la lectura,
	// pues habíamos tomado un lock de escritura.
	EXPECT_EQ(0, eventos.size());

	// Libero el lock de escritura y espero que termine el thread.
	lock.wunlock();
	pthread_join(reader, NULL);

	// Verifico que el thread ahora sí haya podido leer.
	EXPECT_EQ(LECTURA, eventos[0]);
}

///////////////////////////////////////////////////////////////////////////////
// TEST: No ocurre inanición de escritura                                    //
///////////////////////////////////////////////////////////////////////////////

TEST_F(RWLockTest, NoOcurreInanicionDeEscritura) {
	// Este test verifica que si hay al menos un read lock tomado al momento
	// de solicitarse un write lock, ocurre que:
	//
	// 1. El proceso que solicita el write lock queda bloqueado.
	// 2. Luego que dicho proceso queda bloqueado, cualquier otro proceso que
	//    solicita read lock también queda bloqueado.
	// 3. El proceso del punto 1. se desbloquea cuando se liberan los read
	//    locks que se encontraban tomados originalmente.
	// 4. Los procesos del punto 2. se desbloquean después que el proceso del
	//    punto 1. libera su write lock.
	//
	// De esta manera nos aseguramos que todas las solicitudes de write lock
	// bloqueadas eventualmente se desbloqueen, por más que mientras que éstas
	// permanecen bloqueadas se solicite un número de read locks tan grande
	// como uno quiera.

	// Imprimo estimativo del tiempo de ejecución del test.
	int t = NUM_TESTS_DE_INANICION * TIEMPO_DE_ESPERA * 2 / 1000000;
	cout << "Demora aproximadamente " << t << " segundos...\r" << flush;

	for(int i = 0; i < NUM_TESTS_DE_INANICION; i++) {
		// Obtengo lock de lectura.
		lock.rlock();

		// Creo un thread que intenta escribir y le doy tiempo a ejecutarse.
		pthread_t writer;
		pthread_create(&writer, NULL, write, this);
		esperar();

		// Verifico que el thread que intenta escribir haya quedado bloqueado,
		// pues habíamos tomado un lock de lectura.
		EXPECT_EQ(0, eventos.size()) << eventos_to_str();

		// Creo un thread que intenta leer y le doy tiempo a ejecutarse.
		pthread_t reader;
		pthread_create(&reader, NULL, read, this);
		esperar();

		// Verifico que el thread que intenta leer haya quedado bloqueado, pues
		// la solicitud de write lock del thread anterior quedó bloqueada, y
		// cualquier solicitud posterior de read lock debería quedar bloqueada
		// hasta que se libere el write lock del proceso que escribe.
		EXPECT_EQ(0, eventos.size()) << eventos_to_str();

		// Libero el lock de lectura y espero que terminen los threads.
		lock.runlock();
		pthread_join(writer, NULL);	
		pthread_join(reader, NULL);

		// Verifico que primero se haya ejecutado el thread que escribe y luego
		// el thread que lee, en ése orden.
		EXPECT_EQ(ESCRITURA, eventos[0]) << eventos_to_str();
		EXPECT_EQ(LECTURA, eventos[1])   << eventos_to_str();

		// Limpio el vector de eventos para poder repetir el test.
		eventos.clear();
	}
}

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}