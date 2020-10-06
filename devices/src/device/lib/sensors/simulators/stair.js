// Staircase simulator

Logger = require("../../../../lib/logger")
Simulator = require("./simulator")

module.exports = class Stair extends Simulator {
	constructor (config, value, opts) {
		const { step_interval, min, max } = opts

		// call super constructor
		super(config, value, opts)

		// configuration
		this.step_interval = step_interval

		// Integer values
		this.min = min
		this.max = max

		// Start ascending staircase
		this.ascend = true
	}

	// Function: update
	// Description:
	//   Update the value as staircase, ascending to max,
	//   and then descending to min, and so on
	update() {
		if (this.ascend) {
			this._value += this.step_interval
			if (this._value >= this.max) {
				this._value = this.max
				this.ascend = false
			}
		} else {
			this._value -= this.step_interval
			if (this._value <= this.min) {
				this._value = this.min
				this.ascend = true
			}
		}
		this.logger.debug("Simulator Stair update value: " + this._value)
		return this._value
	}

	// Function: valid
	// Description:
	//   Check if value is within min/max range
	valid(value) {
		if (value < this.min || value > this.max) return false
		return true
	}
}
