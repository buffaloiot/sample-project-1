/**
 * This handler class maps input values to output values
 *
 * Configuration:
 *  {
 *    "type": "state",       // this handler type
 *    "state_count": {       // State count map
 *      0: 5                 // Number of times state needs to be observed
 *                           // before transitioning states to this state
 *    },
 *    "state": {             // state map
 *      0: 33                // key = input value, value = output value
 *    }
 *  }
 */

#include "handlers.hpp"
#include "handlers/state.hpp"
#include <jsoncpp/json/json.h>
#include <mosquitto.h>
#include <iostream>

using namespace std;

State::State(string name, mosquitto *client, appConfig *config) : Handlers(name, client, config)
{
	// Setup type of handler
	type = HandlerTypes::topic;

	// Get configuration values
	if (hconfig.isMember("state")) {
		for(Json::Value::const_iterator it=hconfig["state"].begin(); it != hconfig["state"].end(); ++it) {
			state[stoi(it.key().asString())] = it->asInt();
		}
	}
	if (hconfig.isMember("state_count")) {
		for(Json::Value::const_iterator it=hconfig["state_count"].begin(); it != hconfig["state_count"].end(); ++it) {
			state_count[stoi(it.key().asString())] = it->asInt();
		}
	}
}

void State::handleTopic(string topic, string msg)
{
	// verify this instance handles topic
	if (topic == subTopic) {
		// check message against current state
		int current_state = stoi(msg);
		if (current_state != last_state) {
			// Get count for current state
			auto count = state_count.find(current_state);
			if (count != state_count.end()) {
				// state count configuration exists, check if satisified
				if (++last_count < count->second) {
					// did not exceed limit
					return;
				}
			}
			// Get value for current state
			auto value = state.find(current_state);
			if (value != state.end()) {
				publish(to_string(value->second));
			}
			else {
				cerr << "ERROR [State] invalid state received: " << current_state << endl;
			}

			// Successfully changed states
			last_state = current_state;
		}
		else {
			// Always reset last count if the state hasn't changed
			last_count = 0;
		}
	}
}
