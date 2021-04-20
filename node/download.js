const requestLight = require("@onivim/request-light")

const fs = require("fs")

const url = process.argv[2]
const destPath = process.argv[3]
const file = fs.createWriteStream(destPath)

requestLight
    .xhr({
        type: "get",
        url,
        writeStream: file,
    })
    .then((response) => {
        if (response.status == 200) {
            process.stdout.write(`Download of ${url} was successful.`)
        } else {
            process.stdout.write(`Download of ${url} failed - status code: {response.status}`)
        }
    })
