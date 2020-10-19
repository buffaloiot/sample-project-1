/**
 *  Main
 */

#include "config.hpp"
#include "mqtt.hpp"
#include <iostream>

using namespace std;

appConfig *Config;

/**
 *  Function: main
 *  Description:
 *    Application start point
 */
int main (int argc, char **argv)
{
	// get application configuration
	Config = process_env();

	// Start MQTT Client
	start_mqtt();

	// Cleanup config
	delete Config;

	cout << "INFO Exiting..." << endl;
	return 0;
}
