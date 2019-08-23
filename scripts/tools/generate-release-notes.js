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
    return `- [#${commit.shortCommitId}](https://github.com/onivim/oni2/commit/${commit.commitId}): ${commit.message} (__@[${commit.author}](https://github.com/${commit.author})__)`;
};

const commitsToMarkdown = (commits) => {
    return commits.map(commitToMarkdown).join("\n");
};

const date = new Date();
const oneDayAgo = new Date(date.setDate(date.getDate() - 1));

const oneWeekAgo = new Date(date.setDate(date.getDate() - 7));

const getActivePrsForRepo = async (repo) => {
    return axios.get(`https://api.github.com/repos/${repo}/pulls`, { headers: { 'User-Agent': 'onivim2' } })
    .then((response) => {
    const body = response.data;

    
    return body.map((pr) => {
    
        return {
            repoName: repo,
            repoUrl: "https://github.com/" + repo,
            number: pr.number,
            prUrl: pr.html_url,
            author: pr.user.login,
            date: new Date(pr.updated_at),
            title: pr.title,
        };
      }).filter((pr) => {
        const prUpdateTime = pr.date.getTime();
        const weekAgoTime = oneWeekAgo.getTime();
        return prUpdateTime >= weekAgoTime;
      });
    })
    .catch((err) => {
        console.error(err.toString());
    });

};

const prToMarkdown = (pr) => {
    return `- [${pr.repoName}](${pr.repoUrl}) [#${pr.number}](${pr.prUrl}): ${pr.title} ([@${pr.author}](https://github.com/${pr.author}))`
};

const prsToMarkdown = (prs) => {
    return prs.map(prToMarkdown).join("\n");
};


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
    const dayReleaseCommits = await Promise.all(dayCommits.map((c) => getCommitInfo(c)));
    //const dayReleaseCommits = [];
    console.log(dayReleaseCommits);

    const weekReleaseCommits = await Promise.all(weekCommits.map((c) => getCommitInfo(c)));
    //const weekReleaseCommits = [];

    const dayReleaseNotes = commitsToMarkdown(dayReleaseCommits);
    console.log("Daily release notes:");
    console.log("====================");
    console.log(dayReleaseNotes);
    
    const weekReleaseNotes = commitsToMarkdown(weekReleaseCommits);
    console.log("Week release notes:");
    console.log("====================");
    console.log(weekReleaseNotes);


    const oniPrs = await getActivePrsForRepo("onivim/oni2");
    const libvimPrs = await getActivePrsForRepo("onivim/libvim");
    const reasonLibvimPrs = await getActivePrsForRepo("onivim/reason-libvim");
    const treeSitterPrs = await getActivePrsForRepo("onivim/esy-tree-sitter");
    const reasonTreeSitterPrs = await getActivePrsForRepo("onivim/reason-tree-sitter");
    const reveryPrs = await getActivePrsForRepo("revery-ui/revery");

    let prs = [].concat(oniPrs).concat(libvimPrs).concat(reasonLibvimPrs).concat(treeSitterPrs).concat(reasonTreeSitterPrs).concat(reveryPrs);

    prs = prs.sort((a, b) => {
        return b.date.getTime() - a.date.getTime();
    });

    const prReleaseNotes = prsToMarkdown(prs);

    console.log("Active PR notes:");
    console.log("====================");
    console.log(prReleaseNotes);

    //TODO:
    //dayReleaseNotes = "- #abcdef - fix blah (@bryphe)\n- #abcde2 - fix blah2(@bryphe)";
    //const weekReleaseNotes = "- #abcdef - fix blah (@bryphe)\n- #abcde2 - fix blah2(@bryphe)";

    console.log("Writing release notes to release.json...");
    fs.writeFileSync("release.json", JSON.stringify({
        commitId,
        dayReleaseNotes,
        weekReleaseNotes,
        prReleaseNotes,
    }, null, 2));
    console.log("Done!");
};

run();

