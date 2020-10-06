// Thermostat Sensor

SimulatorFactory = require("./simulators")
Sensor = require("./sensor")

module.exports = class Thermostat extends Sensor {
	constructor (config, opts) {
		const { sensorConfig } = opts

		// call super constructor
		super(config, opts)

		// Setup speed simulator
		var simulatorConfig = sensorConfig.simulator
		this.temp = sensorConfig.temp || 0
		this.target = sensorConfig.target || 0
		var createSimulator = SimulatorFactory(config)
		this.simulator = createSimulator(simulatorConfig.type, this.temp, this.target, simulatorConfig)
	}

	// Function: loop
	// Description:
	//   Publish new temperature value
	loop() {
		this.client.publish(this.pubTopic, this.temp.toString())
		this.logger.info("[" + this.name + "] Publishing Temperature: " + this.temp + " to topic: " + this.pubTopic)
		if (this.simulator) this.temp = this.simulator.update()
	}

	// Function: handleTopic
	// description:
	//   Handle message received
	handleTopic(topic, msg) {
		this.logger.debug("[" + this.name + "] sensor handleTopic: topic = " + topic + ", msg = " + msg.toString())
		if (topic === this.subTopic) {
			var target = parseInt(msg)
			var valid = true
			if (this.simulator) valid = this.simulator.valid(target)
			if (isNaN(target) || !valid) {
				this.logger.error("Invalid target temperature received: " + msg)
			} else {
				this.target = target
				if (this.simulator) this.simulator.target = target
				this.logger.info("Setting target temperature: " + target)
			}
		}
	}
}
