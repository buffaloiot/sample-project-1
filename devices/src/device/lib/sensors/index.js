// Create Sensor Factory

Logger = require("../../../lib/logger")

module.exports = (config) => {

	// Setup logger
	logger = Logger(config)

	return (type, ...args) => {
		// don't allow blacklisted types
		if (["index", "utils", "sensor"].includes(type)) return

		try {
			sensor = require("./" + type)
		} catch(err) {
			logger.error("Could not create sensor '" + type + "': " + err)
			return
		}
		return new sensor(config, ...args)
	}
}
