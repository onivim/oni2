const cson = require('cson');
const fs = require('fs');

const sourceFile = process.argv[2];
const destFile = process.argv[3];

console.log(`Converting ${sourceFile} -> ${destFile}...`);

const parsed = cson.load(sourceFile);
fs.writeFileSync(destFile, JSON.stringify(parsed, null, 2), "utf-8");
