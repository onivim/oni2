/* 
    generate-release-notes.js

    Helper script to generate release notes for each build uploaded to the portal
 */

const cp = require("child_process");
const fs = require("fs");
const http = require("http");
const https = require("https");

const commitId = process.argv[2];

if (!commitId) {
    console.log("Must specify commit id for release");
    process.exit();
}

const getCommitAtDate = (date) => {
    const month = date.toLocaleString('en-us', { month: 'short' });
    const day = date.getDate();
    const year = date.getFullYear();
    const ret = cp.execSync(`git rev-list -1 --before="${month} ${day} ${year}" master`).toString("utf-8");
    return ret.trim();
};

const getCommitsForRange = (startCommit, endCommit) => {
    const ret = cp.execSync(`git log ${startCommit}..${endCommit} --format="%H"`).toString("utf-8");
    return ret.trim().split("\n").map(s => s.trim());
};

const getCommitInfo = async (commit) => {
    return axios.get('https://api.github.com/repos/onivim/oni2/commits', { headers: { 'User-Agent': 'onivim2' } })
    .then((response) => {
    console.log("Got response: " + response);
        return {
            author: response.author.login,
            committer: response.committer.login,
            message: response.message.split('\n')[0],
        };
    })
    .catch((err) => {
        console.error(err);
    });
};


const date = new Date();
const oneDayAgo = new Date(date.setDate(date.getDate() - 1));
const oneWeekAgo = new Date(date.setDate(date.getDate() - 7));

const dayAgoCommit = getCommitAtDate(oneDayAgo);
const weekAgoCommit = getCommitAtDate(oneWeekAgo);

console.log("Previous commit: " + getCommitAtDate(oneDayAgo));
console.log("Week previous commit: " + getCommitAtDate(oneWeekAgo));

const axios = require('axios');

console.log("Commits in the past day: ");
const dayCommits = getCommitsForRange(dayAgoCommit, commitId);
console.log(dayCommits);

console.log("Commits in the past week: ");
const weekCommits = getCommitsForRange(weekAgoCommit, commitId);
console.log(weekCommits);

// Active work

const run = async () => {
    //const dayReleaseNotes = await Promise.all(dayCommits.map((c) => getCommitInfo(c)));
    //console.log(dayReleaseNotes);

    //const weekReleaseNotes = await Promise.all(weekCommits.map((c) => getCommitInfo(c)));
    //console.log(weekReleaseNotes);

    // TODO:
    const dayReleaseNotes = "- #abcdef - fix blah (@bryphe)\n- #abcde2 - fix blah2(@bryphe)";
    const weekReleaseNotes = "- #abcdef - fix blah (@bryphe)\n- #abcde2 - fix blah2(@bryphe)";

    const activePrs = "- PR #636: Added syntax highlighting for golang (@NinoFurger)\n- PR #611: UI: Initial message / notifications\n";

    console.log("Writing release notes to release.json...");
    fs.writeFileSync("release.json", JSON.stringify({
        dayReleaseNotes,
        weekReleaseNotes,
        activePrs
    }));
    console.log("Done!");
};

run();

