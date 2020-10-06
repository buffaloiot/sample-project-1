// Temperature Simulator Class
// Temperature changes based on target tempaterature

Logger = require("../../../../lib/logger")
Simulator = require("./simulator")

module.exports = class Temp extends Simulator {
	constructor (config, value, target, opts) {
		const { step_interval, incr_threshold, decr_threshold, decr_min, incr_max } = opts

		// call super constructor
		super(config, value, opts)

		// configuration
		this.step_interval = step_interval

		// Set target temperature
		this._target = target

		// Integer values defining the min/max temperature around target temperature
		this.decr_min = decr_min
		this.incr_max = incr_max
		this.min = target - decr_min
		this.max = target + incr_max

		// value between 0 and 1 defining the threshold at which randomizer
		// will change the current value if in the min/max range
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
	//   Update the value
	update() {
		if (this._value > this.max) {
			// perform descending staircase
			this._value -= this.step_interval
			if (this._value <= this.min) this._value = this.min
		} else if (this._value < this.min) {
			// perform ascending staircase
			this._value += this.step_interval
			if (this._value >= this.max) this._value = this.max
		} else {
			// we are in range, perform random value in range in steps of 1
			var random = Math.random()
			if (random <= this.decr_threshold) {
				this._value -= 1
				if (this._value <= this.min) this._value = this.min
			} else if (random >= this.incr_threshold) {
				this._value += 1
				if (this._value >= this.max) this._value = this.max
			}
		}
		this.logger.debug("Simulator Thermostat update value: " + this._value + " for target: " + this._target)
		return this._value
	}

	// Setters
	set target(value) {
		this._target = value
		this.min = value - this.decr_min
		this.max = value + this.incr_max
	}
}
