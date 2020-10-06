// MQTT Module

const Logger = require("./logger")
const Mqtt = require("mqtt")

module.exports = config => {

	logger = Logger(config)

	return new Promise((resolve, reject) => {
		const mqttConfig = config.mqtt

		options = {
			keepalive: 0,
			connectTimeout: mqttConfig.connectTimeout
		}
		const url = "mqtt://" + mqttConfig.host + ":" + mqttConfig.port
		const client = Mqtt.connect(url, options)
		logger.debug('MQTT url: ' + url)
		client.on('connect', () => {
			// Connection established
			resolve(client)
		})

		client.on('error', (err) => {
			reject("MQTT connection: " + err);
		})
	})
}
