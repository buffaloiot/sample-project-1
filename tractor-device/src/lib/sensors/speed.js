// Speed Sensor

Logger = require("../logger")
utils = require("./utils")

module.exports = class Speed {
	constructor (config, client) {
		this.config = config
		this.client = client
		this.type = "speed"

		// Setup configuration
		var device = config && config.device || {},
			attr = device.sensors && device.sensors.speed || {}
		this.attr = {
			start: attr.start || 0,
			rate: attr.rate || 5,
			max: attr.max || 60,
			interval: attr.interval || device.interval
		}

		// Setup sensor value
		this.speed = this.attr.start
		this.accelerate = true

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
	//   Publish new speed value 
	_loop() {
		this.client.publish(this.pubTopic, this.speed.toString())
		this.logger.debug("Publishing Speed: ", this.speed, " to topic: ", this.pubTopic)
		this._updateSpeed()
	}

	// Function: _updatedSpeed
	// Description:
	//   Update speed to a new value based on rate and max
	_updateSpeed() {
		if (this.accelerate) {
			this.speed += this.attr.rate
			if (this.speed >= this.attr.max) {
				this.speed = this.attr.max
				this.accelerate = false
			}
		} else {
			this.speed -= this.attr.rate
			if (this.speed <= 0) {
				this.speed = 0
				this.accelerate = true
			}
		}
	}
		
	//
	// Public
	//

	// Function: start
	// Description:
	//   The emulated speed sensor will start publishing on start at the interval specified
	start() {
		this.logger.debug('Starting Speed Sensor')

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
			if (err) this.logger.error("Speed subscription failed for topic: ", this.subTopic, ", with error: ", err)
		})
	}

	// Function: stop
	// Description
	//   Stop the emulated speed sensor
	stop() {
		clearInterval(this.loopTimer)
		loopTimer = null
		this.speed = this.attr.start
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
		this.logger.debug("speed sensor handleTopic: topic = ", topic, ", msg = ", msg.toString())
		if (topic === this.subTopic) {
			var speed = parseInt(msg)
			if (isNaN(speed)) {
				this.logger.error("Invalid speed received: ", msg)
			} else {
				this.speed = speed
			}
		}
	}
}
