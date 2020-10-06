// Door Sensor

SimulatorFactory = require("./simulators")
Sensor = require("./sensor")

module.exports = class Door extends Sensor {
	constructor (config, opts) {
		const { sensorConfig } = opts

		// call super constructor
		super(config, opts)

		// Setup door simulator
		var simulatorConfig = sensorConfig.simulator
		this.state = sensorConfig.state || 0
		var createSimulator = SimulatorFactory(config)
		this.simulator = createSimulator(simulatorConfig.type, this.state, simulatorConfig)
	}

	// Function: loop
	// Description:
	//   Publish new door value 
	loop() {
		this.client.publish(this.pubTopic, this.state.toString())
		this.logger.info("[" + this.name + "] Publishing Door State: " + this.state + " to topic: " + this.pubTopic)
		if (this.simulator) {
			this.state = this.simulator.update()
			if (this.state !== 0) this.state = 1
		}
	}

	// Function: handleTopic
	// description:
	//   Handle message received
	handleTopic(topic, msg) {
		this.logger.debug("[" + this.name + "] sensor handleTopic: topic = " + topic + ", msg = " + msg.toString())
		if (topic === this.subTopic) {
			var state = parseInt(msg)
			if (state !== 0) state = 1
			var valid = true
			if (this.simulator) valid = this.simulator.valid(state)
			if (isNaN(state) || !valid) {
				this.logger.error("Invalid door state received: " + msg)
			} else {
				this.state = state 
				if (this.simulator) this.simulator.value = state
				this.logger.info("Setting door state: " + state)
			}
		}
	}
}
