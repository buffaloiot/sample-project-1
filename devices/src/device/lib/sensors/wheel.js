// Wheel Sensor

SimulatorFactory = require("./simulators")
Sensor = require("./sensor")

module.exports = class Wheel extends Sensor {
	constructor (config, opts) {
		const { sensorConfig } = opts

		// call super constructor
		super(config, opts)

		// Setup wheel position sensor
		var simulatorConfig = sensorConfig.simulator
		this.position = sensorConfig.position || 0
		this.min_position = sensorConfig.min_position || 0
		this.max_position = sensorConfig.max_position || 0
		var createSimulator = SimulatorFactory(config)
		this.simulator = createSimulator(simulatorConfig.type, this.position, simulatorConfig)
	}

	// Function: loop
	// Description:
	//   Publish new wheel position value 
	loop() {
		this.client.publish(this.pubTopic, this.position.toString())
		this.logger.info("[" + this.name + "] Publishing Wheel Position: " + this.position + " to topic: " + this.pubTopic)
		if (this.simulator) this.position = this.simulator.update()
	}

	// Function: handleTopic
	// description:
	//   Handle message received
	handleTopic(topic, msg) {
		this.logger.debug("[" + this.name + "] wheel position sensor handleTopic: topic = " + topic + ", msg = " + msg)
		if (topic === this.subTopic) {
			var wheel_position = parseInt(msg)
			var valid = true
			if (this.simulator) valid = this.simulator.valid(wheel_position)
			if (isNaN(wheel_position) || !valid || wheel_position > this.max_position || wheel_position < this.min_position) {
				this.logger.error("Invalid wheel position received: " + msg)
			} else {
				this.position = wheel_position
				if (this.simulator) this.simulator.value = wheel_position
				this.logger.info("Setting wheel position: " + wheel_position)
			}
		}
	}
}
