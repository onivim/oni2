const followRedirects = require("follow-redirects")

// The default for `follow-redirects` is to limit the body to 10 MB...
// which isn't large enough for big extensions available on open-vsx.
followRedirects.maxBodyLength = 128 * 1024 * 1024 // 1024 MB

const http = followRedirects.http
const https = followRedirects.https
const fs = require("fs")

const url = new URL(process.argv[2])
const destPath = process.argv[3]

let requestFn = null

if (url.protocol == "http:") {
    requestFn = http.get
} else if (url.protocol == "https:") {
    requestFn = https.get
} else {
    throw "Unrecognized protocol: " + url.protocol
}

const file = fs.createWriteStream(destPath)

requestFn(url, (response) => {
    response.pipe(file)
})
