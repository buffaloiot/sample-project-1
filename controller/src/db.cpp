/**
 ** Thread 1 - DB
 **/

#include <iostream>
#include <unistd.h>

using namespace std;

void *init_db(void *t) {
	cout << "==> [db] Intialize DB Client" << endl;
	sleep(1);
	pthread_exit(NULL);
}
