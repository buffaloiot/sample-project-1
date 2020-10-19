/**
 *  SAMPLE APPLICATION
 *
 *  A simple IoT controller application that receives messages from a MQTT Topic
 *  and based on the topic format `<location>/<device>/<device ID>/<sensor>` will
 *  store the sensor reading in the configured PostgreSQL DB endpoint.   The DB
 *  schema is simply:
 *
 *    CREATE USER sample;
 *    ALTER USER sample WITH PASSWORD 'password';
 *    CREATE DATABASE sample WITH OWNER sample;
 *    GRANT ALL PRIVILEGES ON DATABASE sample TO sample;
 *    GRANT CONNECT ON DATABASE sample to sample;
 *
 *    \connect sample sample
 *
 *    CREATE TABLE readings (
 *      location text,
 *      device_type text,
 *      device_id text,
 *      sensor text,
 *      ts timestamp with time zone,
 *      reading integer
 *    );
 *
 *  In addition, for a set of configured sensor handlers, a factory pattern
 *  was used to instantiate handlers from configuration that perform
 *  predefined business logic.
 *
 *  Three predefined handlers were included that send command values to a device
 *  subscribing on the handler's publish topic:
 *  - hysteresis: If a sensor value exceeds a maximum value, a max output value is
 *                sent; otherwise, if the sensor value drops below a minimum value,
 *                a min value is sent.
 *  - state: For a configured state map, a sensor value matching a state will send
 *           a value configured for that state.  A `state_count` map sets the number
 *           of times that state is seen consecutively before a value is sent for that
 *           state.  A value is only sent once for that state change.
 *  - scheduler: For a configured schedule, a specific value is sent for the
 *               configured time.
 *
 *  In this sample applications the following architecture applies:
 *
 *  [tractor device] --------> MQTT Broker --------> [tractor controller (instance of this application)]
 *  [warehouse device] ---/                  \-----> [warehouse controller (instance of this application)]
 *
 *  Requires:
 *     - minimum g++ 8 for c++20 features
 *     - libs:
 *       - libmosquitto-dev
 *       - libjsoncpp-dev
 *       - libpqxx-dev
 *       - libpq-dev
 *
 *  Compiling the code
 *    $ g++ -o controller controller.cpp -L lib -lmosquitto -lpqxx -lpq -ljsoncpp -pthread
 *
 *  Example controller application run:
 *    $ MQTT_HOSTNAME=172.17.0.1 MQTT_PORT=1883 PG_CONNECTION_STRING=postgresql://sample:password@172.17.0.1:5432/sample \
 *    $ MQTT_KEEPALIVE_INTERVAL=60 MQTT_SUB_TOPIC=# HANDLER_CONFIG_FILE=config/all.json ./controller
 *
 *  all.json:
 *     {
 *       "engine_overheat": {
 *         "type": "hysteresis",
 *         "subTopic": "farm/tractor/device1/temp",
 *         "pubTopic": "farm/tractor/device1/cmd/speed",
 *         "hysteresis": {
 *           "max": {
 *             "limit": 40,
 *             "value": 0,
 *             "repeat": true
 *           },
 *           "min": {
 *             "limit": 30,
 *             "value": 60,
 *             "repeat": false
 *           }
 *         }
 *       },
 *       "wheel_position": {
 *         "type": "scheduler",
 *         "pubTopic": "farm/tractor/devices1/cmd/wheel_position",
 *         "interval": 1,
 *         "max": 60,
 *         "schedule": {
 *           "0": -45,
 *           "20": 0
 *         }
 *       },
 *       "door_open": {
 *         "type": "state",
 *         "subTopic": "farm/warehouse/device2/door_state",
 *         "pubTopic": "farm/warehouse/device2/cmd/temp",
 *         "state_count": {
 *           "0": 5,
 *           "1": 1
 *         },
 *         "state": {
 *           "0": 0,
 *           "1": -10
 *         }
 *       }
 *     }
 */

#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <limits>
#include <map>
#include <mosquitto.h>
#include <pqxx/pqxx>
#include <sstream>
#include <string.h>
#include <thread>
#include <vector>

using namespace std;

/*
 * Application Configuration
 */

// Structures
typedef struct {
	string mqtt_hostname;
	unsigned short int mqtt_port;
	string mqtt_sub_topic;
	unsigned int mqtt_keepalive_interval;
	string pg_connection;
	Json::Value handlers;
} appConfig;

// configuration object
extern appConfig *Config;

// Functions
extern appConfig *process_env(void);
extern string get_env(std::string const&, std::string const);

/**
 * Handler Factory
 */

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

/**
 * DB
 */

// Functions
extern void insert_reading(appConfig*, const char*, const char*, const char*, const char*, int);

/**
 * MQTT Client
 */

// Functions
extern void start_mqtt();
extern mosquitto *create_mqtt_client(vector<Handlers *>*);
extern void mqtt_subscription_handler(struct mosquitto*, void*, const struct mosquitto_message*);
extern void init_handlers(vector<Handlers *>&, mosquitto*);
extern void start_timer_handlers(vector<Handlers *>&);

/**
 *  Hysteresis Handler
 */

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

/**
 *  Schedule Handler Header
 */

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

/**
 *  State Handler Header
 */

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

///////////
// SOURCE
///////////

/**
 *  Main
 */

appConfig *Config;

/**
 *  Function: main
 *  Description:
 *    Application start point
 */
int main (int argc, char **argv)
{
	// get application configuration
	Config = process_env();

	// Start MQTT Client
	start_mqtt();

	// Cleanup config
	delete Config;

	cout << "INFO Exiting..." << endl;
	return 0;
}


/**
 * MQTT Client Functions
 */

/**
 *  Function: start_mqtt
 *  Description:
 *	  Start MQTT Client using the application configuration object
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
 *	  Create a MQTT Client instance and start connection
 *  Args:
 *    handlers - pointer to a list of handlers
 *  Returns:
 *    mosquitto - mosquitto client object
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
 *	  Handle all subscription messages and start handler logic based on topic stucture of
 *	  <location>/<device>/<device ID>/<sensor>
 *  Args:
 *    mosq - mosquitto client object
 *    obj - handlers object as configured with mosquitto_new
 *    message - received message
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
 *   Initialize all configured handler plugins using Factory Pattern and add
 *   them to the list of handlers.  Handlers may be topic or timer based.
 * Args:
 *   handlers - reference to vector object of handlers
 *   client - misquitto client object
 */
void init_handlers(vector<Handlers *> &handlers, mosquitto *client)
{
	// iterate over configure iterators
	for(Json::Value::const_iterator it=Config->handlers.begin(); it != Config->handlers.end(); ++it) {
		string name(it.key().asString());
		Json::Value hconfig = Config->handlers[name];
		// Factory Create handler
		handlers.push_back(Handlers::makeHandler(hconfig["type"].asString(), name, client, Config));
	}
}

/**
 * Function: start_timer_handlers
 * Description:
 *   Start any timer based handlers
 * Args:
 *   handlers - reference to a list of handlers
 */
void start_timer_handlers(vector<Handlers *> &handlers)
{
	// iterate over all handlers
	for (int idx = 0; idx < handlers.size(); idx++) {
		if (handlers[idx]->getType() == HandlerTypes::timer) {
			cout << "INFO [handlers] Starting timer handler: " << handlers[idx]->getName() << endl;

			// Start timer
			// Get interval in seconds, defaults 1
			Json::Value *jsonInterval = handlers[idx]->getValue("interval");
			unsigned int interval = jsonInterval ? jsonInterval->asInt() : 1;
			// convert to milliseconds
			interval *= 1000;

			Handlers *handler = handlers[idx];
			// start scheduler handler as a new thread using Lambda capature syntax
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

/**
 * DB Insert Functions
 */

/**
 *  Function: insert_reading
 *  Description:
 *	  Handle writing device readings to DB
 *  Args:
 *    config - application configuration
 *    location - device location
 *    device_type - type of device
 *    device_id - id of device
 *    sensor - name of sensor
 *    reading - sensor reading to store
 */
void insert_reading(appConfig *config, const char *location, const char *device_type, const char *device_id, const char *sensor, int reading)
{
	// Mark insert with a timestamp
	long int ts = static_cast<long int> (std::time(0));
	cout << "DEBUG [insert] Prepare readings for location: " << location << ", device_type: " << device_type << ", device_id: " << device_id << ", sensor: " << sensor << ", ts: " << ts << ", reading: " << reading << endl;

	try
	{
		// Create new PG connection
		pqxx::connection connection{config->pg_connection};

		// Prepare Reading
		std::string sql = "INSERT INTO readings(location, device_type, device_id, sensor, ts, reading) VALUES ($1, $2, $3, $4, to_timestamp($5), $6)";
		connection.prepare("readings_insert", sql);

		// Perform transaction
		pqxx::work transaction{connection};
		transaction.exec_prepared("readings_insert", location, device_type, device_id, sensor, ts, reading);
		transaction.commit();
	}
	catch (pqxx::sql_error const &e)
	{
		// Handle SQL errors
		std::cerr << "ERROR [insert] SQL: " << e.what() << std::endl;
		return;
	}
	catch (std::exception const &e)
	{
		// Handle other errors
		std::cerr << "ERROR [insert] Other: " << e.what() << std::endl;
		return;
	}
}

/**
 * Config Functions supporting application configuration data
 */

/**
 * Function: get_env
 * Description:
 *   Get env value for environment variable name.   If environment variable
 *   is unset, the default value is used.
 * Args:
 *   key - environment variable name
 *   def - default value
 * Returns:
 *   string - value set for environment variable
 */
string get_env(std::string const& key, std::string const def = "")
{
	char const* val = getenv(key.c_str());
	return val == NULL ? string(def) : string(val);
}

/**
 * Function: process_env
 * Description:
 *   Get configuration from environment variables
 * Returns:
 *   appConfig - application configuration data
 */
appConfig *process_env(void)
{
	// Allocate a new config object
	appConfig *config = new appConfig;

	cout << "INFO processing environment variables" << endl;

	// get all environment variables
	config->mqtt_hostname = get_env("MQTT_HOSTNAME", "localhost");
	config->mqtt_port = stoi(get_env("MQTT_PORT", "1883"));
	config->pg_connection = get_env("PG_CONNECTION_STRING");
	config->mqtt_keepalive_interval = stoi(get_env("MQTT_KEEPALIVE_INTERVAL", "60"));
	config->mqtt_sub_topic = get_env("MQTT_SUB_TOPIC", "#");

	// get handler configuration from HANLDERS as JSON string
	Json::Reader reader;
	string handlerConfigFile = get_env("HANDLER_CONFIG_FILE");
	if (handlerConfigFile.length()) {
		// Read configuration JSON file
		ifstream ifs(handlerConfigFile);
		reader.parse(ifs, config->handlers);
	}
	else {
		// Read configuration from JSON string
		string handlerConfig = get_env("HANDLER_CONFIG", "{}");
		reader.parse(handlerConfig, config->handlers);
	}

	return config;
}

/**
 * Handler Factory
 */

/**
 * Handlers Class Static Member Function: makeHandler
 * Description:
 *   Handlers factory method for creating Handlers type objects from the known
 *   handler plugins.
 * Args:
 *   handler_plugin - type of handler
 *   handler_name - name of handler instance
 *   client - mosquitto client
 *   appConfig - application configuration object
 * Returns:
 *   New handler object
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
 * Args:
 *   name - name of handler instance
 *   client - mosquitto client
 *   appConfig - application configuration object
 */
Handlers::Handlers(string name, mosquitto *client, appConfig *config) : client {client }, name{ name }
{
	// get handler configuration
	hconfig = config->handlers.isMember(name) ? config->handlers[name] : new Json::Value;

	// Get topics configuration if available
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
 * Args:
 *   topic - current MQTT topic for associated message
 *   msg - current message
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
 *   Generic function to publish messages to the instance pubTopic
 * Args:
 *   text - message to publish
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
 * Returns:
 *   Handler name
 */
string Handlers::getName()
{
	return name;
}

/**
 * Handlers Class Member Function: getType
 * Description:
 *   returns the type of handler
 * Returns:
 *   topic or timer
 */
HandlerTypes::type Handlers::getType()
{
	return type;
}

/**
 * Handlers Class Member Function: getValue
 * Description:
 *   Get JSON Value from handler config
 * Args:
 *   key - configuration property to get value for
 * Returns:
 *   json value object
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
 * Args:
 *   key - configuration property to get value for
 * Returns:
 *   json value object
 */
Json::Value *Handlers::getMapValue(string key, string idx)
{
	if (hconfig.isMember(key))
		return &hconfig[key][idx];
	else
		return nullptr;
}

/**
 * This handler performs a hysteresis function on the handled value.
 * If max is hit, set the value for max until min is hit.
 * If min is hit, set the value for min until max is hit.
 * If repeat is true, set the value on every handle value until hysteresis switches states
 * If repeat is false, set the value once for a new hysteresis state
 *
 * Configuration:
 *  {
 *	"type": "hysteresis",		// this handler type
 *	"hysteresis": {			  // hysteresis states
 *	  "max": {				   // upper limit
 *		"limit": 100,			// if greater then limit, upper limit is set.
 *		"value": 0,			  // value to publish on triggering upper limit
 *		"repeat": true		   // repeat until lower limit is triggered
 *	  },
 *	  "min": {				   // lower limimt
 *		"limit": 50,			 // if less then limit, lower limit is set
 *		"value": 10			  // value to publish on trigger lower limit
 *		"repeat": false		  // repeat until upper limit is triggered
 *	  }
 *	}
 *  }
 */

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

/**
 * This handler class publishes values for the specified schedule
 *
 * Configuration:
 *  {
 *	"type": "scheduler",   // this handler type
 *	"interval": 1,		 // interval in seconds
 *	"max": 10,			 // max number of intervals
 *	"schedule": {		  // schedule to perform
 *	  0: 1,				// at interval 0 set the value 1
 *	  5: 0				 // at interval 5 set the value 0
 *	},
 *	"pubTopic": "foo"	  // publish new value to this publish topic
 *  }
 */

/**
 * Scheduler Handler Class Member Function: Scheduler
 * Description:
 *   Scheduler Constructor
 * Args:
 *   name - name of handler instance
 *   client - mosquitto client
 *   appConfig - application configuration object
 */
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

/**
 * Schduler Handler Class Member Function: handleTimeout
 * Description:
 *   Called when instance interval tick occurs
 */
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

/**
 * This handler class maps input values to output values
 *
 * Configuration:
 *  {
 *	"type": "state",	   // this handler type
 *	"state_count": {	   // State count map
 *	  0: 5				 // Number of times state needs to be observed
 *						   // before transitioning states to this state
 *	},
 *	"state": {			 // state map
 *	  0: 33				// key = input value, value = output value
 *	}
 *  }
 */

/**
 * State Handler Class Member Function: State
 * Description:
 *   State Constructor
 * Args:
 *   name - name of handler instance
 *   client - mosquitto client
 *   appConfig - application configuration object
 */
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

/**
 * State Handler Class Member Function: handleTopic
 * Description:
 *   Process topic and message.  Ignore if not configured to handle.
 *   Supports only explicit topic matches.
 * Args:
 *   topic - current MQTT topic for associated message
 *   msg - current message
 */
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
