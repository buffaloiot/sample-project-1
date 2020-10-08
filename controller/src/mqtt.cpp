/**
 ** Thread 2 - MQTT
 **/

#include <iostream>
#include <unistd.h>

using namespace std;

void *init_mqtt(void *t) {
	cout << "==> [mqtt] Intialize MQTT Client" << endl;
	sleep(1);
	pthread_exit(NULL);
}

