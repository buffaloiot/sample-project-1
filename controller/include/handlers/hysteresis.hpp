#pragma once

/**
 *  Hysteresis Handler Header
 */

#include "handlers.hpp"
#include <mosquitto.h>

class Hysteresis : public Handlers
{
	public:
		Hysteresis(string, mosquitto*, appConfig*);
		// check if handled topic
		void handleTopic(string topic, string msg);

	private:
		// Hysteresis info
		struct HInfo
		{
			int limit;
			int value;
			bool repeat;
		};
		HInfo min = {0, 0, false};
		HInfo max = {0, 0, false};

		// State information, initial is no state
		enum state { no_state, min_state, max_state};
		state current_state = no_state;
};
