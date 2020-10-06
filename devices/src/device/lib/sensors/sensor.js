// Base Sensor Class

Logger = require("../../../lib/logger")
utils = require("./utils")

module.exports = class Sensor {
	constructor (config, opts) {
		// Extract options
		const { name, sensorConfig, interval, mqttClient, topicPrefix } = opts

		// Setup instance
		this.name = name
		this.client = mqttClient
		this.interval = interval

		// Get pub/sub topics
		this.pubTopic = utils.buildTopic(topicPrefix.publish, name)
		this.subTopic = utils.buildTopic(topicPrefix.subscribe, name)

		// Setup logger
		this.logger = Logger(config)
	}

	// Function: loop
	// Description:	
	//   Noop
	loop() {
		this.logger.info("[" + this.name + "] Sensor loop noop")
	}

	// Function: start
	// Description:
	//   The emulated sensor will start publishing on start at the interval specified
	start() {
		this.logger.info('Starting ' + this.name + ' Sensor')

		// Stop current interval
		if (this.loopTimer) {
			clearInterval(this.loopTimer)
			this.loopTimer = null
		}

		// Start new interval
		this.loopTimer = setInterval(() => {
			this.loop()
		}, this.interval)

		// Subscribe to commands
		this.client.subscribe(this.subTopic, (err) => {
			if (err) this.logger.error(this.name + " subscription failed for topic: " + this.subTopic + ", with error: " + err)
		})
	}

	// Function: stop
	// Description
	//   Stop the emulated sensor
	stop() {
		clearInterval(this.loopTimer)
		loopTimer = null
	}

	// Function: isHandledTopic
	// Description:
	//   Check if topic matches the subscription topic
	isHandledTopic(topic) {
		return topic === this.subTopic
	}

	// Function: handleTopic
	// description:
	//   Handle topic messages
	handleTopic(topic, msg) {
		this.logger.warn("[" + this.name + "] sensor message ignored: " + msg.toString() + " on topic " + topic)
	}
}
