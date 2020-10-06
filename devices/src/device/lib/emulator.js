// device emulator application

const Logger = require("../../lib/logger")
const Config = require("../../lib/config")
const Mqtt = require("../../lib/mqtt")
const SensorFactory = require("./sensors")

module.exports = args => {

	logger = Logger(args)

	Config(args)
		.then(config => {
			logger.debug("config: ", JSON.stringify(config, null, 4))
			Mqtt(config)
				.then(client => {
					logger.info("MQTT connected")

					// Get list of sensors for this device
					device = config.device || {}
					deviceSensors = device.sensors || {}

					// Initialize sensor factory
					createSensor = SensorFactory(config)

					// Initialize sensor instance list
					sensors = {}
					started = false

					for (const name in deviceSensors) {
						sensorConfig = deviceSensors[name]

						// If sensor is enabled, start it
						if (sensorConfig.enabled) {
							opts = {
								name,
								sensorConfig,
								interval: device.interval || 1,
								mqttClient: client,
								topicPrefix: device.topicPrefix || {}
							}
							sensor = createSensor(sensorConfig.type, opts)
						
							if (sensor) {
								sensors[name] = sensor
								sensor.start()
								started = true
							}	
						}
					}

					// If no sensors are available, exit
					if (!started) {
						logger.error("No sensors were started")
						process.exit(3)
					}

					// Setup handler to process subscriptions
					client.on("message", (topic, msg) => {
						// hand-off command to sensor handler
						for (const name in deviceSensors) {
							if (sensors[name] && sensors[name].isHandledTopic(topic)) {
								// Sensor handles this received type of message
								sensors[name].handleTopic(topic, msg)
							}
						}
					})
				}).catch(err => logger.error(err))
		}).catch(err => logger.error(err))
}
