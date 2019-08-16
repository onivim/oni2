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
    return axios.get('https://api.github.com/repos/onivim/oni2/commits/' + commit, { headers: { 'User-Agent': 'onivim2' } })
    .then((response) => {
   // console.log(response.data);
    const body = response.data;
    
        return {
            shortCommitId: commit.substring(0, 8),
            commitId: commit,
            date: new Date(body.commit.author.date),
            author: body.author.login,
            committer: body.committer.login,
            message: body.commit.message.split('\n')[0],
        };
    })
    .catch((err) => {
        console.error(err.toString());
    });
};

const commitToMarkdown = (commit) => {
    return `- [#${commit.shortCommitId}](https://github.com/onivim/oni2/commits/${commit.commitId}): ${commit.message} (@${commit.author})`;
};

const commitsToMarkdown = (commits) => {
    return commits.map(commitToMarkdown).join("\n");
};

const getActivePrsForRepo = async (repo) => {
    return axios.get(`https://api.github.com/repos/onivim/${repo}/pulls`, { headers: { 'User-Agent': 'onivim2' } })
    .then((response) => {
    const body = response.data;

    return body.map((pr) => {
    
        return {
            repoName: repo,
            repoUrl: "https://github.com/onivim/" + repo,
            number: pr.number,
            prUrl: pr.html_url,
            author: pr.user.login,
            date: new Date(pr.updated_at),
            author: body.author.login,
            title: body.title,
        };
      });
    })
    .catch((err) => {
        console.error(err.toString());
    });

};

const prToMarkdown = (pr) => {
    return `- [${pr.repoName}](${pr.repoUrl}) [#${pr.number}](${pr.prUrl}): ${pr.title} (@${pr.author})`
};

const prsToMarkdown = (prs) => {
    return prs.map(prToMarkdown).join("\n");
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
    let dayReleaseCommits = await Promise.all(dayCommits.map((c) => getCommitInfo(c)));
    console.log(dayReleaseCommits);

    const weekReleaseCommits = await Promise.all(weekCommits.map((c) => getCommitInfo(c)));

    const dayReleaseNotes = commitsToMarkdown(dayReleaseCommits);
    console.log("Daily release notes:");
    console.log("====================");
    console.log(dayReleaseNotes);
    
//    const weekReleaseNotes = commitsToMarkdown(weekReleaseCommits);
    const weekReleaseNotes = [];
    console.log("Week release notes:");
    console.log("====================");
    console.log(weekReleaseNotes);

    const prs = await getActivePrsForRepo("oni2");
    const prReleaseNotes = prsToMarkdown(prs);

    // TODO:
    //dayReleaseNotes = "- #abcdef - fix blah (@bryphe)\n- #abcde2 - fix blah2(@bryphe)";
//     const weekReleaseNotes = "- #abcdef - fix blah (@bryphe)\n- #abcde2 - fix blah2(@bryphe)";

    console.log("Writing release notes to release.json...");
    fs.writeFileSync("release.json", JSON.stringify({
        dayReleaseNotes,
        weekReleaseNotes,
        prReleaseNotes,
    }));
    console.log("Done!");
};

run();

