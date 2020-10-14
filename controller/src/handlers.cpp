/**
 * Handler Factory
 */

#include "config.hpp"
#include "handlers.hpp"
#include "handlers/scheduler.hpp"
#include "handlers/hysteresis.hpp"
#include "handlers/state.hpp"
#include <iostream>
#include <jsoncpp/json/json.h>
#include <mosquitto.h>

using namespace std;

//
// Handlers Class
//

/**
 * Handlers Class Static Member Function: makeHandler
 * Description:
 *   Handlers factory method for creating Handlers type objects from the known
 *   handler plugins.
 */
Handlers *Handlers::makeHandler(string handler_plugin, string handler_name, mosquitto *client, appConfig *config)
{
	// Simple lookup for now
	if (handler_plugin == "scheduler") {
		cout << "INFO [handlers] Creating Scheduler instance: name = " << handler_name << endl;
		return new Scheduler(handler_name, client, config);
	}
	else if (handler_plugin == "hysteresis") {
		cout << "INFO [handlers] Creating Hysteresis instance: name = " << handler_name << endl;
		return new Hysteresis(handler_name, client, config);
	}
	else if (handler_plugin == "state") {
		cout << "INFO [handlers] Creating State instance: name = " << handler_name << endl;
		return new State(handler_name, client, config);
	}

	// Handler not found
	cerr << "ERROR [handlers] Invalid handler type: " << handler_plugin << endl;
	return nullptr;
}

/**
 * Handlers Class Member Function: Handlers
 * Description:
 *   Handlers Constructor
 */
Handlers::Handlers(string name, mosquitto *client, appConfig *config) : client {client }, name{ name }
{
	// get handler configuration
	hconfig = config->handlers.isMember(name) ? config->handlers[name] : new Json::Value;

	// Get topics if available
	if (hconfig.isMember("pubTopic")) {
		// Publish topic is set
		pubTopic = hconfig["pubTopic"].asString();
	}
	if (hconfig.isMember("subTopic")) {
		// Subscription topic is set
		subTopic = hconfig["subTopic"].asString();
	}
}

/**
 * Handlers Class Member Function: handleTopic
 * Description:
 *   Stub for topic based handlers
 */
void Handlers::handleTopic(string topic, string msg)
{
	cout << "DEBUG [handlers] handleTopic stub" << endl;
}

/**
 * Handlers Class Member Function: handleTimeout
 * Description:
 *   Stub for timer based handlers
 */
void Handlers::handleTimeout(void)
{
	cout << "DEBUG [handlers] handleTimeout stub" << endl;
}

/**
 * Handlers Class protected Member Function: publish
 * Description:
 *   Stub for timer based handlers
 */
void Handlers::publish(string text)
{
	int ret;

	cout << "DEBUG [Handlers] Publishing value: " << text
		<< ", for topic: " << pubTopic
		<< endl;
	ret = mosquitto_publish(client, NULL, pubTopic.c_str(), text.length(), text.c_str(), 0, false);
	if (ret) cerr << "ERROR [Handlers] Can't publish to Mosquitto server: " << ret << endl;
}

/**
 * Handlers Class Member Function: getName
 * Description:
 *   returns the name of handler
 */
string Handlers::getName()
{
	return name;
}

/**
 * Handlers Class Member Function: getType
 * Description:
 *   returns the type of handler, topic or timer
 */
HandlerTypes::type Handlers::getType()
{
	return type;
}

/**
 * Handlers Class Member Function: getValue
 * Description:
 *   Get JSON Value from handler config
 */
Json::Value *Handlers::getValue(string key)
{
	if (hconfig.isMember(key))
		return &hconfig[key];
	else
		return nullptr;
}

/**
 * Handlers Class Member Function: getMapValue
 * Description:
 *   Get JSON Value from map for a given index
 */
Json::Value *Handlers::getMapValue(string key, string idx)
{
	if (hconfig.isMember(key))
		return &hconfig[key][idx];
	else
		return nullptr;
}

