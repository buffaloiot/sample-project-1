// device emulator application

const Logger = require("./logger")
const Config = require("../config")
const Mqtt = require("./mqtt")

module.exports = args => {

	logger = Logger(args)

	Config(args)
		.then(config => {
			logger.debug("config: ", JSON.stringify(config, null, 4))
			Mqtt(config)
				.then(client => {
					logger.debug("MQTT connected")
					// Start Sensors
					sensors = {}
					started = false

					sensorTypes = Object.keys(config.device.sensors)
					sensorTypes.forEach(sensorType => {
						let Sensor;

						try {
							Sensor = require("./sensors/" + sensorType)
							started = true
						} catch(err) {
							logger.error("Could not load sensor library: " + sensorType + ", " + err)
						}

						// If no sensors are available, exit
						if (!started) {
							logger.error("No sensors were started")
							process.exit(3)
						}

						if (Sensor) {
							sensor = sensors[sensorType] = new Sensor(config, client)
							sensor.start()
						}
					})

					// Setup handler to process subscriptions
					client.on("message", (topic, msg) => {
						// hand-off command to sensor handler
						sensorTypes.forEach(sensorType => {
							if (sensors[sensorType] && sensors[sensorType].isHandledTopic(topic)) {
								// Sensor handles this received type of message
								sensors[sensorType].handleTopic(topic, msg)
							}
						})
					})
				}).catch(err => logger.error(err))
		}).catch(err => logger.error(err))
}
