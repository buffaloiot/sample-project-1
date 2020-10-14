/**
 * This handler class publishes values for the specified schedule
 *
 * Configuration:
 *  {
 *    "type": "scheduler",   // this handler type
 *    "interval": 1,         // interval in seconds
 *    "max": 10,             // max number of intervals
 *    "schedule": {          // schedule to perform
 *      0: 1,                // at interval 0 set the value 1
 *      5: 0                 // at interval 5 set the value 0
 *    },
 *    "pubTopic": "foo"      // publish new value to this publish topic
 *  }
 */

#include "handlers.hpp"
#include "handlers/scheduler.hpp"
#include <jsoncpp/json/json.h>
#include <mosquitto.h>
#include <iostream>

using namespace std;

Scheduler::Scheduler(string name, mosquitto *client, appConfig *config) : Handlers(name, client, config)
{
	// Setup type of handler
	type = HandlerTypes::timer;

	// Init
	tick = 0;

	// Get configuration values
	interval = hconfig.isMember("interval") ? hconfig["interval"].asInt() : 1;
	max = hconfig.isMember("max") ? hconfig["max"].asInt() : 1;
	if (hconfig.isMember("schedule")) {
		for(Json::Value::const_iterator it=hconfig["schedule"].begin(); it != hconfig["schedule"].end(); ++it) {
			schedule[stoi(it.key().asString())] = it->asInt();
		}
	}
}

void Scheduler::handleTimeout(void)
{
	// Get value for interval value
	auto value = schedule.find(tick);
	if (value != schedule.end()) {
		// Found value so publish value
		publish(to_string(value->second));
	}

	// Increment tick for next interval
	tick += interval;
	if (tick >= max) tick = 0;
}
