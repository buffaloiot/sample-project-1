// Tractor Device Emulator

const cli = require("../lib/cli")
const emulator = require("./lib/emulator")

module.exports = cli

// Start emulator
if (require.main === module) {
	cli(process.argv.slice(2), emulator)
}
