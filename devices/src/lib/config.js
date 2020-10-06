// Read configuration

configPath = __dirname + "/../config"
module.exports = args => {
	return new Promise((resolve, reject) => {
		const { configFile, logLevel } = args
		if (configFile.startsWith("/")) {
			configFilePath = configFile
		} else {
			configFilePath = [configPath, configFile].join("/")
		}
		if (!configFilePath.endsWith(".json")) configFilePath += ".json"
		try {
			config = require(configFilePath)
		} catch(err) {
			reject("Could not get configuration: " + err)
		}
		config.logLevel = logLevel
		resolve(config)
	})
}
