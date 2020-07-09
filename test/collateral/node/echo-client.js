const net = require("net");

const log = (msg) => console.log(`[NODE] ${msg}`)

const pipe = process.argv[2];
log("Connected to pipe: " + pipe);

const client = net.connect(pipe, () => {
	log("Connected.");
});

let currentId = 0;

client.on('data', (buffer) => {
	let type = buffer.readUInt8(0);
	let id = buffer.readUInt32BE(1);
	let ack = buffer.readUInt32BE(5);
	let length = buffer.readUInt32BE(9);
	
	let data = buffer.slice(13, buffer.length);
	console.log(`Got data - type: ${type} id: ${id} ack: ${ack} length: ${length} - data: |${data.toString("utf8")}|`);

	let header = Buffer.alloc(13);
	header.writeUInt8(1, 0);
	header.writeUInt32BE(currentId++, 1);
	header.writeUInt32BE(ack, 5);
	header.writeUInt32BE(length, 9);

	let outBuffer = Buffer.concat([header, data]);
	console.log(`Sending data - total length: ${outBuffer.length}`);
	client.write(outBuffer)
});

client.on('close', () => {
	log("close");
	client.destroy();
});
