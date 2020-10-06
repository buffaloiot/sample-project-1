// Random Simulator Class

Logger = require("../../../../lib/logger")
Simulator = require("./simulator")

module.exports = class Random extends Simulator {
	constructor (config, value, opts) {
		const { step_interval, min, max, incr_threshold, decr_threshold } = opts

		// call super constructor
		super(config, value, opts)

		// configuration
		this.step_interval = step_interval || 1

		// integer values defining range of valid values
		this.min = min || 0
		this.max = max || 5

		// value between 0 and 1 defining the threshold at which randomizer
		// will change the current value.
		//
		// decr_threshold - if a random value between 0 and 1 is less then
		//                  threshold, decrement value by step interval
		// incr_threshold - if a random value between 0 and 1 is greater then
		//                  threshold, increment value by step interval
		//
		// Note for overlapping thresholds, decrement will take precedence
		// Note for gaps in threshold, a random value falling in that range will
		//      not change the value
		this.incr_threshold = incr_threshold || .5
		this.decr_threshold = decr_threshold || .5
	}

	// Function: update
	// Description:
	//   Update the value randomly at interval specified.
	update() {
		const random = Math.random()
		if (random <= this.decr_threshold) {
			this._value -= this.step_interval
			if (this._value <= this.min) this._value = this.min
		} else if (random >= this.incr_threshold) {
			this._value += this.step_interval
			if (this._value >= this.max) this._value = this.max
		}
		this.logger.debug("Simulator Random update value: " + this._value)
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
