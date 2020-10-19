/**
 * This handler performs a hysteresis function on the handled value.
 * If max is hit, set the value for max until min is hit.
 * If min is hit, set the value for min until max is hit.
 * If repeat is true, set the value on every handle value until hysteresis switches states
 * If repeat is false, set the value once for a new hysteresis state
 *
 * Configuration:
 *  {
 *    "type": "hysteresis",        // this handler type
 *    "hysteresis": {              // hysteresis states
 *      "max": {                   // upper limit
 *        "limit": 100,            // if greater then limit, upper limit is set.
 *        "value": 0,              // value to publish on triggering upper limit
 *        "repeat": true           // repeat until lower limit is triggered
 *      },
 *      "min": {                   // lower limimt
 *        "limit": 50,             // if less then limit, lower limit is set
 *        "value": 10              // value to publish on trigger lower limit
 *        "repeat": false          // repeat until upper limit is triggered
 *      }
 *    }
 *  }
 */

#include "handlers.hpp"
#include "handlers/hysteresis.hpp"
#include <mosquitto.h>

/**
 * Hysteresis Handler Class Member Function: Hysteresis
 * Description:
 *   Hysteresis Constructor
 * Args:
 *   name - name of handler instance
 *   client - mosquitto client
 *   appConfig - application configuration object
 */
Hysteresis::Hysteresis(string name, mosquitto *client, appConfig *config) : Handlers(name, client, config)
{
	// Setup type of handler
	type = HandlerTypes::topic;

	// Check if hysteresis configuration is setup
	if (hconfig.isMember("hysteresis")) {
		Json::Value hysteresis = hconfig["hysteresis"];

		// Check if min is configured
		if (hysteresis.isMember("min")) {
			min.limit = hysteresis["min"]["limit"].asInt();
			min.value = hysteresis["min"]["value"].asInt();
			min.repeat = hysteresis["min"]["repeat"].asBool();
		}

		// Check if max is configured
		if (hysteresis.isMember("max")) {
			max.limit = hysteresis["max"]["limit"].asInt();
			max.value = hysteresis["max"]["value"].asInt();
			max.repeat = hysteresis["max"]["repeat"].asBool();
		}
	}
}

/**
 * Hysteresis Handler Class Member Function: handleTopic
 * Description:
 *   Process topic and message.  Ignore if not configured to handle.
 *   Supports only explicit topic matches.
 * Args:
 *   topic - current MQTT topic for associated message
 *   msg - current message
 */
void Hysteresis::handleTopic(string topic, string msg)
{
	if (topic == subTopic) {
		int value = stoi(msg);

		// Set initial state
		if (current_state == no_state) {
			// If no state, then set the current state based on the current value
			if (value <= min.limit) {
				current_state = min_state;
				publish(to_string(min.value));
			}
			else if (value >= max.limit) {
				current_state = max_state;
				publish(to_string(max.value));
			}

			// Nothing else to do until we get next value
			return;
		}

		// Perform Hysteresis
		if (current_state == min_state) {
			// currently min state
			if (value >= max.limit) {
				// max triggered
				publish(to_string(max.value));
				current_state = max_state;
				return;
			}

			// check if we need to publish again
			if (min.repeat) {
				publish(to_string(min.value));
			}
		}
		else if (current_state == max_state) {
			// currently max state
			if (value <= min.limit) {
				// min triggered
				publish(to_string(min.value));
				current_state = min_state;
				return;
			}

			// check if we need to publish again
			if (max.repeat) {
				publish(to_string(max.value));
			}
		}
	}
}
