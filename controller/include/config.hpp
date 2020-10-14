#pragma once

/**
 * Application Configuration Header
 */

#include <iostream>
#include <jsoncpp/json/json.h>

using namespace std;

// Structures
typedef struct {
	string mqtt_hostname;
	unsigned short int mqtt_port;
	string mqtt_sub_topic;
	unsigned int mqtt_keepalive_interval;
	string pg_connection;
	Json::Value handlers;
} appConfig;

// Global configuration object
extern appConfig *Config;

// Functions
extern appConfig *process_env(void);
extern string get_env(std::string const&, std::string const);
