// Read configuration

module.exports = args => {
	return new Promise((resolve, reject) => {
		const { configFile, debug } = args
		if (configFile.startsWith("/")) {
			configFilePath = configFile
		} else {
			configFilePath = [__dirname, configFile].join("/")
		}
		try {
			config = require(configFilePath)
		} catch(err) {
			reject("Could not get configuration file: " + configFilePath)
		}
		config.debug = debug
		resolve(config)
	})
}
