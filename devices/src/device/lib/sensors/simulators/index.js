// Create Simulator Factory

Logger = require("../../../../lib/logger")

module.exports = (config) => {

	// Setup logger
	logger = Logger(config)

	return (type, ...args) => {
		// don't allow blacklisted types
		if (["index", "simulator"].includes(type)) return

		try {
			simulator = require("./" + type)
		} catch(err) {
			logger.error("Could not create simulator '" + type + "': " + err)
			return
		}
		return new simulator(config, ...args)
	}
}
