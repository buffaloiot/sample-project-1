#pragma once

/**
 * Handler Factory Header
 */

#include "config.hpp"
#include <iostream>
#include <jsoncpp/json/json.h>
#include <mosquitto.h>

using namespace std;

//
// Handlers Class
//

namespace HandlerTypes
{
	enum type { none, topic, timer };
}

class Handlers
{
	public:
		// Factory Method
		static Handlers *makeHandler(string, string, mosquitto*, appConfig*);

		// Functions
		Handlers(string, mosquitto*, appConfig*);
		virtual void handleTopic(string, string);
		virtual void handleTimeout(void);
		HandlerTypes::type getType();
		string getName();
		Json::Value *getValue(string);
		Json::Value *getMapValue(string, string);

	private:
		appConfig *config;

	protected:
		void publish(string);
		HandlerTypes::type type;
		string name;
		Json::Value hconfig;
		mosquitto *client;
		string pubTopic;
		string subTopic;
};
