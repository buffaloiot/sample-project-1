#pragma once

/**
 * MQTT Client Headers
 */

#include "config.hpp"
#include "insert.hpp"
#include <iostream>
#include <mosquitto.h>
#include <string.h>
#include <sstream>

using namespace std;

// Thread main function
extern void start_mqtt();
extern mosquitto *create_mqtt_client();
extern void mqtt_subscription_handler(struct mosquitto*, void*, const struct mosquitto_message*);
