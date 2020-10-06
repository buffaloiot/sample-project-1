// Logger

module.exports = (config) => {

	// context
	logger = {}

	levels = [
		"error",
		"warn",
		"info",
		"debug",
		"trace"
	]

	// current log level
	log_level = levels.indexOf(config.logLevel)
	if (log_level === -1) log_level = 0

	// log wrapper
	logger.log = (func, msg=[]) => {
		msg.unshift(func.toUpperCase() + ": ")
		console[func](...msg)
	}

	// add log levels
	for (const log_level_idx in levels) {
		const level = levels[log_level_idx]
		logger[level] = (...msg) => {
			if (log_level >= log_level_idx) {
				logger.log(level, msg)
			}
		}
	}

	return logger
}
