#pragma once

/**
 *  Schedule Handler Header
 */

#include "handlers.hpp"
#include <iostream>
#include <map>
#include <mosquitto.h>

using namespace std;

class Scheduler: public Handlers
{
	public:
		Scheduler(string, mosquitto*, appConfig*);
		void handleTimeout(void);
	private:
		unsigned int tick;
		unsigned int interval;
		unsigned int max;
		map<int, int> schedule;
};
