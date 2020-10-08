/**
 ** Main Thread Header
 **/

#include "db.h"
#include "mqtt.h"

const static struct {
	const char *name;
	void *(*func)(void *);
} threadTypes [] = {
	{ "DB", init_db },
	{ "MQTT", init_mqtt }
};
