const net = require("net");

const pty = require('node-pty');

process.stdout.write("Hello!");
const log = (msg) => console.log(`[NODE] ${msg}`)

const pipe = process.argv[2];
const cwd = process.argv[3];
const cmd = process.argv[4];
log("Connected to pipe: " + pipe);
log("Using cwd: " + cwd);
log("Using cmd: " + cmd);

const remainingArguments = process.argv.slice(5, process.argv.length);
log("Using remaining arguments: " + JSON.stringify(remainingArguments));

const client = net.connect(pipe, () => {
	log("Connected.");
});

const ptyProcess = pty.spawn(cmd, [], {
	name: 'xterm-color',
	cols: 80,
	rows: 30,
	cwd: cwd,
	env: process.env,
});

let currentId = 0;

const InMessageType = {
	input: 0, // user input (string)
	resize: 1, // json ({rows: ..., cols: ...})
	kill: 2, // n/a
}

const OutMessageType = {
	data: 0, // string
	exit: 1, // json ({exitCode: })
};

const write = (type, str) => {

	let data = Buffer.alloc(str.length, str);
	let length = Buffer.byteLength(data);
	let header = Buffer.alloc(13);

	// Protocol augmentation:
	// The 'ack' field is being used to discriminiate between message types
	const ack = type;

	header.writeUInt8(1, 0);
	header.writeUInt32BE(currentId++, 1);
	header.writeUInt32BE(ack, 5);
	header.writeUInt32BE(length, 9);

	let outBuffer = Buffer.concat([header, data]);
	console.log(`Sending data - total length: ${outBuffer.length}`);
	console.log(`data: ${str}`)
	client.write(outBuffer)
};

ptyProcess.on('data', (data) => write(OutMessageType.data, data));

ptyProcess.on('exit', (exitCode) => {
	write(OutMessageType.exit, JSON.stringify({ exitCode: exitCode }));
	console.log("js: pty exited");
});

client.on('data', (buffer) => {
	console.log("Got input: " + Buffer.toString(buffer));
	let type = buffer.readUInt8(0);
	let id = buffer.readUInt32BE(1);
	let ack = buffer.readUInt32BE(5);
	let length = buffer.readUInt32BE(9);

	// Protocol augmentation:
	// The 'ack' field is being used to discriminate between message types
	// 0: User 

	let data = buffer.slice(13, buffer.length);
	switch (ack) {
		case InMessageType.input: ptyProcess.write(data);
			break;
		case InMessageType.resize:
			const size = JSON.parse(data);
			console.log("DATA: " + data);
			ptyProcess.resize(size.cols, size.rows);
			break;
		case InMessageType.kill:
			ptyProcess.kill();
		default:
			// Unknown message type
			break;
	}
});

client.on('close', () => {
	log("close");
	ptyProcess.close();
	client.destroy();
});
