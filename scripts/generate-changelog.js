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

// PARSING

function parseSubject(str) {
    let result = str.match(/^([a-z]+)(?:\(([^\)]+)\))?:\s*(.*)$/)
    return result
        ? {
              type: result[1],
              scope: result[2],
              subject: result[3],
          }
        : {
              subject: str,
          }
}

function extractPRNumber(str) {
    let result = str.match(/\(#(\d+)\)$/)
    return result && result[1]
}

function parseLogLine(line) {
    return Object.assign(parseSubject(line.substr(20)), {
        hash: line.substr(0, 8),
        time: line.substr(10, 9),
        pr: extractPRNumber(line),
    })
}

// GENERATE NEW ENTRIES

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

function extractLogEntry(body) {
    let result = body.match(/```changelog\r?\n((.|[\s\S])*)\r?\n```/)
    return result && result[1]
}

// GENERATE XML

function createCommitXml({ type, scope, hash, pr, time, content, subject }) {
    const typeAttr = type ? `type="${type}" ` : ""
    const scopeAttr = scope ? `scope="${scope}" ` : ""

    return `  <commit ${typeAttr}${scopeAttr}hash="${hash}" pr=${pr} time=${time}>
    ${content ? content.replace(/\n/g, "\n    ") : subject}
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

Promise.all(
    getLog(lastCommitHash).map((commit) => {
        const fluffTypes = ["test", "docs", "chore"]
        // If the commit has an associated PR and is not "fluff", check the
        // PR for more, and more up-to-date, information
        if (commit.pr && !fluffTypes.includes(commit.type)) {
            console.log(`Checking ${commit.hash} / #${commit.pr}`)
            return getPullRequest(token, commit.pr)
                .then((pr) => {
                    let logEntry = extractLogEntry(pr.body)
                    let parsed = parseSubject(pr.title)
                    return Object.assign({}, commit, parsed, { content: logEntry })
                })
                .catch((error) => {
                    console.error(`Error retrieving PR #${commit.pr} for ${commit.hash}: ${error}`)
                    return Promise.reject(error)
                })
        } else {
            return commit
        }
    }),
)
    .then((commits) => {
        console.log("New commits:", commits)

        const xmlCommits = commits.map(createCommitXml)
        const newXml = addXmlCommits(xml, xmlCommits)

        console.log("Writing to: " + path)

        write(path, newXml)

        console.log("Done.")
    })
    .catch((error) => console.error("Failed:", error))
