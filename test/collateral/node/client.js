const net = require("net");

const log = (msg) => console.log(`[NODE] ${msg}`)

const pipe = process.argv[2];
log("Connected to pipe: " + pipe);

const client = net.connect(pipe, () => {
	log("Connected.");
});

client.on('close', () => {
	log("close");
	client.destroy();
});
