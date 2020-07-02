const fs = require("fs");
const path = require("path");

const commitId = process.argv[2];
const destPath = process.argv[3];
console.log(`Commit: ${commitId}`);
console.log(`Destination Folder: ${destPath}`);

const templateMd = path.join(__dirname, "template.md");
const destinationMd = path.join(destPath, "README.md");

const inputLines = fs.readFileSync(templateMd, "utf8");

const lines = inputLines.replace("%%COMMITID%%", commitId);

console.log(lines)

fs.writeFileSync(destinationMd, lines, "utf8");
