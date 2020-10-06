const minimist = require("minimist");
const help = () => {
	const helpText = `Options:
	--CF, --cf, --config-file=FILE                       Use external JSON configuration file, default: default.json.
	--L, --l, --log-level=error,warn,info,debug,trace    Print logging information to console based on log level, default: info.

	Default options:
	--CF=default.json`

	console.log(helpText)
}

module.exports = (args, cb) => {

	args = minimist(args, {
		string: ["config-file","log-level"],
		boolean: ["debug"],
		alias: {
			configFile: ["CF", "cf", "config-file"],
			logLevel: ["L", "l", "log-level"]
		},
		default: {
			configFile: "default.json",
			logLevel: "info"
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
