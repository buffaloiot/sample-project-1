/**
 * MQTT Client Functions
 */

#include "mqtt.hpp"

/**
 *  Function: start_mqtt
 *  Description:
 *    Start MQTT Client using the application configuration object
 */
void start_mqtt() {

	int ret;
	struct mosquitto *mosq_client;

	cout << "INFO [mqtt] Intialize MQTT Client" << endl;

	// Initialize Mosquitto Library
	mosquitto_lib_init();

	// Start MQTT Client
	mosq_client = create_mqtt_client();

	// Check if a client was created
	if (mosq_client != nullptr) {
		// Create subscription to listen for messages from devices
		ret = mosquitto_subscribe(mosq_client, NULL, Config->mqtt_sub_topic.c_str(), 0);
		if (ret) {
			cerr << "ERROR [mqtt] Can't subscribe to topic " << Config->mqtt_sub_topic << " with error: " << ret << endl;
		} else {
			// Add callback for all incomming messages
			mosquitto_message_callback_set(mosq_client, mqtt_subscription_handler);

			// Wait on mosquitto
			mosquitto_loop_forever(mosq_client, -1, 1);
		}
	}

	// When client calls mosquitto_disconnect(), clean up
	mosquitto_destroy(mosq_client);
	mosquitto_lib_cleanup();

	return;
}

/**
 *  Function: create_mqtt_client
 *  Description:
 *    Create a MQTT Client instance and start connection
 */
mosquitto *create_mqtt_client()
{

	struct mosquitto *mosq = NULL;

	// Create a Mosquitto Runtime instance
	// Use a random client ID
	mosq = mosquitto_new(NULL, true, NULL);

	if (!mosq) {
		cerr << "ERROR [mqtt] Can't initialize Mosquitto library" << endl;
		return mosq;
	}

	// Client initialized, make connection
	int ret = mosquitto_connect(mosq, Config->mqtt_hostname.c_str(), Config->mqtt_port, Config->mqtt_keepalive_interval);
	if (ret) {
		cerr << "ERROR [mqtt] Can't connect to Mosquitto server: mqtt://" << Config->mqtt_hostname << ":" << Config->mqtt_port << endl;
		// Clean up mosquitto instance, and return null pointer to indicate no client was created
		mosquitto_destroy(mosq);
		return nullptr;
	}

	return mosq;
}

/**
 *  Function: mqtt_subscription_handler
 *  Description:
 *    Handle all subscription messages and start handler logic based on topic stucture of
 *    <location>/<device>/<device ID>/<sensor>
 */
void mqtt_subscription_handler(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	// Received message
	cout << "DEBUG [mqtt] Received Topic: " << message->topic << ", Message: " << (char *) message->payload << endl;

	if (strlen(message->topic) && strlen((char *)message->payload)) {
		string location, device, device_id, sensor;
		string topic(message->topic);
		int reading = atoi((char *)message->payload);
		istringstream iss(topic);

		// Get topic tokens
		getline(iss, location, '/');
		getline(iss, device, '/');
		getline(iss, device_id, '/');
		getline(iss, sensor, '/');

		// write device data to DB by calling insert as a detached thread
		thread(insert_reading, Config, location.c_str(), device.c_str(), device_id.c_str(), sensor.c_str(), reading).detach();

		// hand off message to handler
		// TODO: Add handlers here

	} else {
		cerr << "ERROR [mqtt] Received invalid message" << endl;
	}
}
