// WheelPosition Sensor

Logger = require("../logger")
utils = require("./utils")

module.exports = class WheelPosition {
	constructor (config, client) {
		this.config = config
		this.client = client
		this.type = "wheel_position"

		// Setup configuration
		var device = config && config.device || {},
			attr = device.sensors && device.sensors.wheel_position || {}
		this.attr = {
			start: attr.start || 0,
			max: attr.max || 45,
			min: attr.min || -45,
			interval: attr.interval || device.interval
		}

		// Setup sensor value
		this.wheel_position = this.attr.start

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
	//   Publish new wheel position value 
	_loop() {
		this.client.publish(this.pubTopic, this.wheel_position.toString())
		this.logger.debug("Publishing WheelPosition: ", this.wheel_position, " to topic: ", this.pubTopic)
	}

	//
	// Public
	//

	// Function: start
	// Description:
	//   The emulated wheel position sensor will start publishing on start at the interval specified
	start() {
		this.logger.debug('Starting Wheel Position Sensor')

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
			if (err) this.logger.error("Wheel Position subscription failed for topic: ", this.subTopic, ", with error: ", err)
		})
	}

	// Function: stop
	// Description
	//   Stop the emulated wheel position sensor
	stop() {
		clearInterval(this.loopTimer)
		loopTimer = null
		this.wheel_position = this.attr.start
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
		this.logger.debug("wheel position sensor handleTopic: topic = ", topic, ", msg = ", msg)
		if (topic === this.subTopic) {
			var wheel_position = parseInt(msg)
			if (isNaN(wheel_position) || wheel_position > this.attr.max || wheel_position < this.attr.min) {
				this.logger.error("Invalid wheel position received: ", msg)
			} else {
				this.wheel_position = wheel_position
			}
		}
	}
}
