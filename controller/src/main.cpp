/**
 *  Main
 */

#include "main.hpp"

appConfig *Config;

int main (int argc, char **argv)
{
	int rc;

	// get application configuration
	Config = process_env();

	// Start MQTT Client
	start_mqtt();

	// Cleanup config
	delete Config;

	cout << "INFO Exiting..." << endl;
	return 0;
}
