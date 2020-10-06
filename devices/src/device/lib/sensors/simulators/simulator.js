// Base Simulator Class

Logger = require("../../../../lib/logger")

module.exports = class Simulator {
	constructor (config, value, opts) {
		this.config = config
		this._value = value

		this.logger = Logger(config)
	}

	// Function: update
	// Description:
	//   Update the value
	update() {
		this.logger.debug("Simulator update: noop")
		return this._value
	}

	// Function: valid
	// Description:
	//   Test if value is valid
	valid(value) {
		return true
	}

	// Setters
	set value(value) {
		this._value = value
	}
}
