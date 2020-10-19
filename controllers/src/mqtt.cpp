/**
 * MQTT Client Functions
 */

#include "mqtt.hpp"
#include "config.hpp"
#include "handlers.hpp"
#include "insert.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <mosquitto.h>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>

using namespace std;

/**
 *  Function: start_mqtt
 *  Description:
 *    Start MQTT Client using the application configuration object
 */
void start_mqtt()
{
	int ret;
	struct mosquitto *mosq_client;
	vector<Handlers *> handlers;

	cout << "INFO [mqtt] Intialize MQTT Client" << endl;

	// Initialize Mosquitto Library
	mosquitto_lib_init();

	// Start MQTT Client
	mosq_client = create_mqtt_client(&handlers);

	// Check if a client was created
	if (mosq_client != nullptr) {
		// Initialize Handlers
		init_handlers(handlers, mosq_client);

		// Create timer based handlers
		start_timer_handlers(handlers);

		// Setup subscription last
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
mosquitto *create_mqtt_client(vector<Handlers *> *handlers)
{
	struct mosquitto *mosq = NULL;

	// Create a Mosquitto Runtime instance
	// Use a random client ID
	mosq = mosquitto_new(NULL, true, (void *) handlers);

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
	vector<Handlers *> *handlers = (vector<Handlers *> *) obj;

	// Received message
	if (strlen(message->topic) && strlen((char *)message->payload)) {
		int reading = atoi((char *)message->payload);
		string topic(message->topic);
		string location, device_type, device_id, sensor;

		istringstream iss(topic);

		// Get topic tokens
		getline(iss, location, '/');
		getline(iss, device_type, '/');
		getline(iss, device_id, '/');
		getline(iss, sensor, '/');

		// write device data to DB
		insert_reading(Config, location.c_str(), device_type.c_str(), device_id.c_str(), sensor.c_str(), reading);

		// hand off message to handler
		for (int idx = 0; idx < handlers->size(); idx++) {
			Handlers *handler = (*handlers)[idx];
			if (handler->getType() == HandlerTypes::topic) {
				// Process message with topic handler
				handler->handleTopic(string(message->topic), string((char *)message->payload));
			}
		}

	} else {
		cerr << "ERROR [mqtt] Received invalid message" << endl;
	}
}

/**
 * Function: init_handlers
 * Description:
 *   Initialize all configured handler plugins and add them to the list of handlers.
 *   Handlers may be topic or timer based.
 */
void init_handlers(vector<Handlers *> &handlers, mosquitto *client)
{
	// iterate over configure iterators
	for(Json::Value::const_iterator it=Config->handlers.begin(); it != Config->handlers.end(); ++it) {
		string name(it.key().asString());
		Json::Value hconfig = Config->handlers[name];
		handlers.push_back(Handlers::makeHandler(hconfig["type"].asString(), name, client, Config));
	}
}

/**
 * Function: start_timer_handlers
 * Description:
 *   Start any timer based handlers
 */
void start_timer_handlers(vector<Handlers *> &handlers)
{
	// iterate over all handlers
	for (int idx = 0; idx < handlers.size(); idx++) {
		if (handlers[idx]->getType() == HandlerTypes::timer) {
			cout << "INFO [handlers] Starting timer handler: " << handlers[idx]->getName() << endl;

			// Start timer
			// Get interval in milliseconds
			Json::Value *jsonInterval = handlers[idx]->getValue("interval");
			unsigned int interval = jsonInterval ? jsonInterval->asInt() : 1;
			interval *= 1000;
			Handlers *handler = handlers[idx];
			thread([handler, interval]() {
				while (true) {
					auto next_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
					handler->handleTimeout();
					std::this_thread::sleep_until(next_time);
				}
			}).detach();
		}
	}
}
