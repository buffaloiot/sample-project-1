/**
 * Config
 *
 * Functions to support application configuration data
 */

#include "config.hpp"
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>

using namespace std;

/**
 * Function: get_env
 * Description:
 *   Get env value for key
 */
string get_env(std::string const& key, std::string const def = "")
{
	char const* val = getenv(key.c_str());
	return val == NULL ? string(def) : string(val);
}

/**
 * Function: process_env
 * Description:
 *   Get configuration from environment variables
 * Returns:
 *   appConfig*
 */
appConfig *process_env(void)
{
	// Allocate a new config object
	appConfig *config = new appConfig;

	cout << "INFO processing environment variables" << endl;

	// get all environment variables
	config->mqtt_hostname = get_env("MQTT_HOSTNAME", "localhost");
	config->mqtt_port = stoi(get_env("MQTT_PORT", "1883"));
	config->pg_connection = get_env("PG_CONNECTION_STRING");
	config->mqtt_keepalive_interval = stoi(get_env("MQTT_KEEPALIVE_INTERVAL", "60"));
	config->mqtt_sub_topic = get_env("MQTT_SUB_TOPIC", "#");

	// get handler configuration from HANLDERS as JSON string
	Json::Reader reader;
	string handlerConfigFile = get_env("HANDLER_CONFIG_FILE");
	if (handlerConfigFile.length()) {
		// Read configuration JSON file
		ifstream ifs(handlerConfigFile);
		reader.parse(ifs, config->handlers);
	}
	else {
		// Read configuration from JSON string
		string handlerConfig = get_env("HANDLER_CONFIG", "{}");
		reader.parse(handlerConfig, config->handlers);
	}

	return config;
}
