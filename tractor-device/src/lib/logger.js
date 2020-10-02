// Logger

module.exports = (config) => {

	// context
	logger = {}

	// log wrapper
	logger.log = (func, msg=[]) => {
		msg.unshift(func.toUpperCase() + ": ")
		console[func](...msg)
	}

	// add log levels
	logger.debug = (...msg) => {
		if (config.debug) {
			logger.log('debug', msg)
		}
	}

	logger.error = (...msg) => {
		logger.log('error', msg)
	}

	return logger
}
