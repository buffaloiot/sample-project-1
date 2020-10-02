// EngineTemp Sensor

Logger = require("../logger")
utils = require("./utils")

module.exports = class EngineTemp {
	constructor (config, client) {
		this.config = config
		this.client = client
		this.type = "engine_temp"

		// Setup configuration
		var device = config && config.device || {},
			attr = device.sensors && device.sensors.engine_temp|| {}
		this.attr = {
			start: attr.start || 0,
			min: Array.isArray(attr.range) && attr.range[0] || 0,
			max: Array.isArray(attr.range) && attr.range[1] || 100,
			interval: attr.interval || device.interval
		}

		// Setup sensor value
		this.engine_temp = this.attr.start

		// Get pub/sub topics
		var mqtt = config && config.mqtt || {}
		this.pubTopic = utils.buildTopic(mqtt.pubTopicPrefix, this.type)
		this.subTopic = utils.buildTopic(mqtt.subTopicPrefix, this.type)

		// Setup logger
		this.logger = Logger(config)
	}

	//
	// Private
	//

	// Function: _loop
	// Description:
	//   Publish new engine temp value 
	_loop() {
		this.client.publish(this.pubTopic, this.engine_temp.toString())
		this.logger.debug("Publishing Engine Temp: ", this.engine_temp, " to topic: ", this.pubTopic)
		this._updateEngineTemp()
	}

	// Function: _updatedEngineTemp
	// Description:
	//   Update engine temperature to a new value based on min/max
	_updateEngineTemp() {
		var random = Math.random()
		if (random < .3) {
			this.engine_temp -= 1
			if (this.engine_temp < this.attr.min) this.engine_temp = this.attr.min
		} else if (random > .5) {
			this.engine_temp += 1
			if (this.engine_temp > this.attr.max) this.engine_temp = this.attr.max
		}	
	}
		
	//
	// Public
	//

	// Function: start
	// Description:
	//   The emulated engine temperature sensor will start publishing on start at the interval specified
	start() {
		this.logger.debug('Starting Engine Temp Sensor')

		// Stop current interval
		if (this.loopTimer) {
			clearInterval(this.loopTimer)
			this.loopTimer = null
		}

		// Start new interval
		this.loopTimer = setInterval(() => {
			this._loop()
		}, this.attr.interval)

		// Subscribe to commands
		this.client.subscribe(this.subTopic, (err) => {
			if (err) this.logger.error("Engine Temp subscription failed for topic: ", this.subTopic, ", with error: ", err)
		})
	}

	// Function: stop
	// Description
	//   Stop the emulated engine temperature sensor
	stop() {
		clearInterval(this.loopTimer)
		loopTimer = null
		this.engine_temp = this.attr.start
	}

	// Function: isHandledTopic
	// Description:
	//   Check if topic matches the subscription topic
	isHandledTopic(topic) {
		return topic === this.subTopic
	}

	// Function: handleTopic
	// description:
	//   Handle message received
	handleTopic(topic, msg) {
		this.logger.debug("engine temp sensor handleTopic: topic = ", topic, ", msg = ", msg)
		if (topic === this.subTopic) {
			var engine_temp = parseInt(msg)
			if (isNaN(engine_temp)) {
				this.logger.error("Invalid engine temperature received: ", msg)
			} else {
				this.engine_temp = engine_temp
			}
		}
	}
}
