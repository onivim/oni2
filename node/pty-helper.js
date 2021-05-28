const net = require("net")

const pty = require("node-pty")

process.stdout.write("Hello!")
let shouldLog = process.env["ONIVIM2_DEBUG_TERMINAL"]
const log = shouldLog ? (msg) => console.log(`[NODE] ${msg}`) : (_msg) => {}

const pipe = process.argv[2]
const cwd = process.argv[3]
const cmd = process.argv[4]
const initialRows = parseInt(process.argv[5])
const initialCols = parseInt(process.argv[6])
log("Connected to pipe: " + pipe)
log("Using cwd: " + cwd)
log("Using cmd: " + cmd)

const remainingArguments = process.argv.slice(7, process.argv.length)
log("Using remaining arguments: " + JSON.stringify(remainingArguments))

const client = net.connect(pipe, () => {
    log("Connected.")
})

const ptyProcess = pty.spawn(cmd, [], {
    name: "xterm-color",
    cols: initialCols || 80,
    rows: initialRows || 30,
    cwd: cwd,
    env: process.env,
    encoding: "utf8",
})

let currentId = 0

const InMessageType = {
    input: 0, // user input (string)
    resize: 1, // json ({rows: ..., cols: ...})
    kill: 2, // n/a
}

const OutMessageType = {
    data: 0, // string
    exit: 1, // json ({exitCode: })
    pidChanged: 2, // int (pid)
    titleChanged: 3, // string (title)
}

const write = (type, str) => {
    let data = Buffer.alloc(str.length, str)
    let length = Buffer.byteLength(data)
    let header = Buffer.alloc(13)

    // Protocol augmentation:
    // The 'ack' field is being used to discriminiate between message types
    const ack = type

    header.writeUInt8(1, 0)
    header.writeUInt32BE(currentId++, 1)
    header.writeUInt32BE(ack, 5)
    header.writeUInt32BE(length, 9)

    let outBuffer = Buffer.concat([header, data])
    client.write(outBuffer)
}

let currentPid = null
let currentTitle = null

ptyProcess.on("data", (data) => {
    write(OutMessageType.data, data)

    if (ptyProcess.process != currentTitle) {
        currentTitle = ptyProcess.process
        log("Title changed: " + ptyProcess.process)
        write(OutMessageType.titleChanged, currentTitle)
    }

    if (ptyProcess.pid != currentPid) {
        currentPid = ptyProcess.pid
        write(OutMessageType.pidChanged, JSON.stringify(currentPid))
        log("Pid changed: " + ptyProcess.pid)
    }
})

ptyProcess.on("exit", (exitCode) => {
    write(OutMessageType.exit, JSON.stringify({ exitCode: exitCode }))
    log("js: pty exited")
})

let pendingData = Buffer.alloc(0)

let HEADER_SIZE = 13

let processPendingPackets = () => {
    while (pendingData.length > HEADER_SIZE) {
        let type = pendingData.readUInt8(0)
        let id = pendingData.readUInt32BE(1)
        let ack = pendingData.readUInt32BE(5)
        let length = pendingData.readUInt32BE(9)

        const packetSize = HEADER_SIZE + length
        // See if the packet is big enough to contain the body
        if (pendingData.length >= packetSize) {
            // It is!
            // Let's grab the packet, and then splice pendingData
            let data = pendingData.slice(HEADER_SIZE, packetSize)
            switch (ack) {
                case InMessageType.input:
                    ptyProcess.write(data)
                    break
                case InMessageType.resize:
                    const size = JSON.parse(data)
                    ptyProcess.resize(size.cols, size.rows)
                    break
                case InMessageType.kill:
                    ptyProcess.kill()
                default:
                    // Unknown message type
                    log("Unknown message type: " + ack)
                    break
            }

            pendingData = pendingData.slice(packetSize)
        } else {
            return
        }
    }
}

client.on("data", (buffer) => {
    pendingData = Buffer.concat([pendingData, buffer])
    processPendingPackets()
})

client.on("close", () => {
    log("close")
    ptyProcess.close()
    client.destroy()
})
