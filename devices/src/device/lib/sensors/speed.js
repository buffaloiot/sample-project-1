// Speed Sensor

SimulatorFactory = require("./simulators")
Sensor = require("./sensor")

module.exports = class Speed extends Sensor {
	constructor (config, opts) {
		const { sensorConfig } = opts

		// call super constructor
		super(config, opts)

		// Setup speed simulator
		var simulatorConfig = sensorConfig.simulator
		this.speed = sensorConfig.speed || 0
		var createSimulator = SimulatorFactory(config)
		this.simulator = createSimulator(simulatorConfig.type, this.speed, simulatorConfig)
	}

	// Function: loop
	// Description:
	//   Publish new speed value 
	loop() {
		this.client.publish(this.pubTopic, this.speed.toString())
		this.logger.info("[" + this.name + "] Publishing Speed: " + this.speed + " to topic: " + this.pubTopic)
		if (this.simulator) this.speed = this.simulator.update()
	}

	// Function: handleTopic
	// description:
	//   Handle message received
	handleTopic(topic, msg) {
		this.logger.debug("[" + this.name + "] sensor handleTopic: topic = " + topic + ", msg = " + msg.toString())
		if (topic === this.subTopic) {
			var speed = parseInt(msg)
			var valid = true
			if (this.simulator) valid = this.simulator.valid(speed)
			if (isNaN(speed) || !valid) {
				this.logger.error("Invalid speed received: " + msg)
			} else {
				this.speed = speed
				if (this.simulator) this.simulator.value = speed
				this.logger.info("Setting speed: " + speed)
			}
		}
	}
}
