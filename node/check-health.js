// check-health.js
// These are Node-side health-checks to validate that
// all required dependencies are available and installed.

const os = require("os")

// TODO: Get exthost working again
// const exthost = require('vscode-exthost');
const sudo = require("sudo-prompt")
const pty = require("node-pty")

/*const shell = os.platform() == 'win32' ? 'cmd.exe' : 'bash';

const ptyProcess = pty.spawn(shell, [], {
	name: 'xterm-color',
	cols: 20,
	rows: 20,
	cwd: process.env.HOME,
	env: process.env
});

ptyProcess.on('data', (data) => {
	console.log("Got data: " + data);
	ptyProcess.destroy();
});

ptyProcess.write('ls\r\n');*/

console.log("Success!")
