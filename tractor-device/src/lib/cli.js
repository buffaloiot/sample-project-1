const minimist = require("minimist");
const help = () => {
	const helpText = `Options:
	--CF, --cf, --config-file=FILE                       Use external JSON configuration file.
	--D, --d, --debug                                    Print debug information to console.

	Default options:
	--CF=default.json`

	console.log(helpText)
}

module.exports = (args, cb) => {

	args = minimist(args, {
		string: ["config-file"],
		boolean: ["debug"],
		alias: {
			configFile: ["CF", "cf", "config-file"],
			debug: ["D", "d", "debug"]
		},
		default: {
			configFile: "default.json",
			debug: false
		},
		unknown: () => {
			console.error("==> Invalid Parameters")
			help()
			process.exit(1)
		}
	});

	// finished processing input supply processed args to callback
	cb(args)
}
