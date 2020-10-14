#pragma once

/**
 *  State Handler Header
 */

#include "handlers.hpp"
#include <iostream>
#include <limits>
#include <map>
#include <mosquitto.h>

using namespace std;

class State : public Handlers
{
	public:
		State(string name, mosquitto*, appConfig *config);
		// handled topic
		void handleTopic(string topic, string msg);

	private:
		int last_state = numeric_limits<int>::max();
		int last_count = 0;
		map<int, int> state;
		map<int, int> state_count;
};
