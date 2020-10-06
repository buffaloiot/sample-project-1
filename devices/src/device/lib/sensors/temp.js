// Temp Sensor

SimulatorFactory = require("./simulators")
Sensor = require("./sensor")

module.exports = class Temp extends Sensor {
	constructor (config, opts) {
		// call super constructor
		super(config, opts)

		// Setup temperature simulator
		var simulatorConfig = sensorConfig.simulator
		this.temp = sensorConfig.temp
		var createSimulator = SimulatorFactory(config)
		this.simulator = createSimulator(simulatorConfig.type, this.temp, simulatorConfig)
	}

	// Function: loop
	// Description:
	//   Publish new temp value 
	loop() {
		this.client.publish(this.pubTopic, this.temp.toString())
		this.logger.info("[" + this.name + "] Publishing Temp: " + this.temp + " to topic: " + this.pubTopic)
		if (this.simulator) this.temp = this.simulator.update()
	}
}
