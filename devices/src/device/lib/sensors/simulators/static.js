// Static Simulator Class

Logger = require("../../../../lib/logger")
Simulator = require("./simulator")

module.exports = class Static extends Simulator {
	constructor (config, value, opts) {
		// call super constructor
		super(config, value, opts)
	}
}
