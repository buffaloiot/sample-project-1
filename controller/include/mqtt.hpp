#pragma once

/**
 * MQTT Client Headers
 */

#include "handlers.hpp"
#include <mosquitto.h>

// Functions
extern void start_mqtt();
extern mosquitto *create_mqtt_client(vector<Handlers *>*);
extern void mqtt_subscription_handler(struct mosquitto*, void*, const struct mosquitto_message*);
extern void init_handlers(vector<Handlers *>&, mosquitto*);
extern void start_timer_handlers(vector<Handlers *>&);
