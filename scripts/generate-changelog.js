const childProcess = require("child_process")

const initialLog = "<changelog>\n<changelog>"

// READ XML

function read(path) {
    const fs = require("fs")
    try {
        return fs.readFileSync(path).toString()
    } catch {
        return null
    }
}

function getLastCommit(xml) {
    let result = xml.match(/<commit[^>]+hash="(.*?)"/)
    return result && result[1]
}

// GENERATE NEW ENTRIES

function extractPRNumber(str) {
    let result = str.match(/\(#(\d+)\)$/)
    return result && result[1]
}

function parseLogLine(line) {
    return {
        hash: line.substr(0, 8),
        time: line.substr(10, 9),
        summary: line.substr(20),
        pr: extractPRNumber(line),
    }
}

function getLog(fromCommit) {
    const range = fromCommit ? fromCommit + ".." : ""
    const command = "git --no-pager log --format='%h %at %s' " + range

    return childProcess
        .execSync(command)
        .toString()
        .split("\n")
        .filter((s) => s)
        .map(parseLogLine)
}

function getPullRequest(token, number) {
    const https = require("https")

    const url = "https://api.github.com/repos/onivim/oni2/pulls/" + number
    const headers = {
        "User-Agent": "oni-fetch",
        Authorization: token && `token ${token}`,
    }

    return new Promise((resolve, reject) =>
        https.get(url, { headers }, (response) => {
            if (response.statusCode == 200) {
                const body = []
                response.on("data", (chunk) => body.push(chunk))
                response.on("end", () => resolve(JSON.parse(body.join(""))))
            } else {
                reject(response.statusCode + ": " + response.statusMessage)
            }
        }),
    )
}

function extractLogEntry(pr) {
    let result = pr.body.match(/```changelog\r?\n((.|[\s\S])*)\r?\n```/)
    return result && result[1]
}

// GENERATE XML

function createCommitXml(commit) {
    return `  <commit hash="${commit.hash}" pr=${commit.pr} time=${commit.time}>
    ${commit.content.replace(/\n/g, "\n    ")}
  </commit>\n`
}

function addXmlCommits(xml, commits) {
    const [before, after] = xml.split(/<changelog>\r?\n/, 2)

    return before + "<changelog>\n" + commits.join("") + after
}

function write(path, content) {
    const fs = require("fs")
    fs.writeFileSync(path, content)
}

// MAIN

let path
let token

process.argv.slice(2).forEach((arg) => {
    if (arg.startsWith("--token=")) {
        token = arg.slice("--token=".length)
    } else if (!arg.startsWith("--")) {
        path = arg
    } else {
        throw new Error("invalid argument: " + arg)
    }
})

const xml = read(path) || initialLog
const lastCommitHash = getLastCommit(xml)

console.log("Retrieving commits since " + lastCommitHash)

const commits = getLog(lastCommitHash)

Promise.all(
    commits
        .filter((commit) => commit.pr)
        .map((commit) => {
            console.log(`Checking ${commit.hash} / #${commit.pr}`)
            return getPullRequest(token, commit.pr)
                .then((pr) => {
                    let logEntry = extractLogEntry(pr)
                    return Object.assign({}, commit, { content: logEntry })
                })
                .catch((error) => {
                    console.error(`Error retrieving PR #${commit.pr} for ${commit.hash}: ${error}`)
                    return Promise.reject(error)
                })
        }),
)
    .then((commits) => {
        const validCommits = commits.filter((commit) => commit && commit.content)

        console.log("New commits:", validCommits)

        const xmlCommits = validCommits.map(createCommitXml)
        const newXml = addXmlCommits(xml, xmlCommits)

        console.log("Writing to: " + path)

        write(path, newXml)

        console.log("Done.")
    })
    .catch((error) => console.error("Failed:", error))
