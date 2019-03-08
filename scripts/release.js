const fs = require("fs-extra");
const path = require("path");

console.log("Working directory: " + process.cwd());

const releaseDirectory = path.join(process.cwd(), "_release");
const platformReleaseDirectory = path.join(releaseDirectory, process.platform);

const extensionsSourceDirectory = path.join(process.cwd(), "extensions");
const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions");

console.log("Creating directory: " + platformReleaseDirectory);

fs.mkdirpSync(platformReleaseDirectory);

// Copy the cur__bin over
let curBin = process.env["cur__bin"];
console.log("Bin folder: " + curBin);

fs.copySync(curBin, platformReleaseDirectory);

// Copy extensions over
fs.copySync(extensionsSourceDirectory, extensionsDestDirectory);
