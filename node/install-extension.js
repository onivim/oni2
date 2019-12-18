const yauzl = require("yauzl");
const fs = require("fs");
const fse = require("fs-extra");
const path = require("path");

const extensionPath = process.argv[2];
const extensionFolder = process.argv[3];
const extensionName = process.argv[4] || path.basename(extensionPath, path.extname(extensionPath));
const extensionVersion = process.argv[5] || "";

const extensionDestFolder = extensionName + extensionVersion;

if (!path.isAbsolute(extensionPath) || !fs.existsSync(extensionPath) || fs.lstatSync(extensionPath).isDirectory()) {
	throw "Extension path must be an absolute path to an extension"
}

if (!path.isAbsolute(extensionFolder) || !fs.lstatSync(extensionFolder).isDirectory()) {
	throw "Extension folder must be an absolute path"
}

const zipFilePath = extensionPath;
const destPath = extensionFolder;

const sourcePathRegex = new RegExp("^extension/");

// From: https://github.com/microsoft/vscode/blob/c2e8b28a9a2396d3f7c1af731fe3cdbca9fa46bf/src/vs/base/node/zip.ts#L51
function modeFromEntry(entry) {
	const attr = entry.externalFileAttributes >> 16 || 33188;

	return [448 /* S_IRWXU */, 56 /* S_IRWXG */, 7 /* S_IRWXO */]
		.map(mask => attr & mask)
		.reduce((a, b) => a + b, attr & 61440 /* S_IFMT */);
}

yauzl.open(zipFilePath, {lazyEntries: true}, (err, zipfile) => {
	if (err) throw err;

	zipfile.readEntry();
	zipfile.on("entry", (entry) => {

		if (!sourcePathRegex.test(entry.fileName)) {
			// Directory file names end with '/'
			// Note that entries for directories themselves are optional
			zipfile.readEntry();
		} else {


			// file entry
			console.log("Opening read stream: " + entry.fileName);
			const fileName = entry.fileName.replace(sourcePathRegex, '');
			const outputDirectory = path.join(destPath, extensionDestFolder);
			const outputPath = path.join(outputDirectory, fileName);
			console.log("Creating directory: " + outputDirectory);
			fse.mkdirpSync(outputDirectory);

			console.log("Filename after mod: " + outputPath);
			zipfile.openReadStream(entry, (err, readStream) => {
				//console.dir(entry);
				if (err) throw err;
				readStream.on("end", () => {
					zipfile.readEntry();
				});
				const mode = modeFromEntry(entry);
				const istream = fs.createWriteStream(outputPath, { mode });
				readStream.pipe(istream);
				istream.once("error", e => { throw e});
				readStream.once("error", e => { throw e});

				zipfile.readEntry();
			})
		}
	})
});

console.log("Install extension");
