/**
 **	Main Thread
 **	-----------
 **
 **	1. Start Mosquitto Client Thread
 **	2. Start PostgreSQL DB Client Thread
 **
 **/

#include "main.h"
#include <iostream>
#include <pthread.h>

using namespace std;

int main (int argc, char **argv)
{
	int rc;

	// Threads, one for each type defined
	pthread_t threads[sizeof(threadTypes) / sizeof(threadTypes[0])];

	// Start DB Client Thread
	for ( size_t i = 0; const auto threadType : threadTypes) {
		cout << "==> Creating " << threadType.name << " Client Thread" << endl;
		rc = pthread_create(&threads[i], NULL, threadType.func, NULL);

		if (rc) {
			cout << "==> Error: Unable to create " << threadType.name << " Thread: " << rc << endl;
			exit(1);
		}
		i++;
	}

	// Wait for child threads to exit before exiting
	for ( size_t i = 0; const auto threadType : threadTypes ) {
		rc = pthread_join(threads[i], NULL);
		if (rc) {
			cout << "==> Error: Unable to join " << threadType.name << " Thread: " << rc << endl;
			exit(2);
		}
		cout << "==> " << threadType.name << " Thread exited" << endl;
		i++;
	}

	cout << "==> Exiting..." << endl;
	pthread_exit(NULL);

	return 0;
}
