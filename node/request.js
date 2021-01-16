const http = require("follow-redirects").http
const https = require("follow-redirects").https

const url = new URL(process.argv[2])

let requestFn = null

if (url.protocol == "http:") {
    requestFn = http.get
} else if (url.protocol == "https:") {
    requestFn = https.get
} else {
    throw "Unrecognized protocol: " + url.protocol
}

const req = requestFn(url, (res) => {
    if (res.statusCode == 200) {
        res.on("data", (d) => {
            process.stdout.write(d)
        })
        res.on("end", () => {
            req.end()
        })
    }
})
