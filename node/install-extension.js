const yauzl = require("yauzl")
const fs = require("fs")
const fse = require("fs-extra")
const path = require("path")

const extensionPath = process.argv[2]
const extensionFolder = process.argv[3]
const extensionDestFolder = process.argv[4]

if (
    !path.isAbsolute(extensionPath) ||
    !fs.existsSync(extensionPath) ||
    fs.lstatSync(extensionPath).isDirectory()
) {
    throw "Extension path must be an absolute path to an extension"
}

if (!path.isAbsolute(extensionFolder) || !fs.lstatSync(extensionFolder).isDirectory()) {
    throw "Extension folder must be an absolute path"
}

const zipFilePath = extensionPath
const destPath = extensionFolder

const sourcePathRegex = new RegExp("^extension/")

// From: https://github.com/microsoft/vscode/blob/c2e8b28a9a2396d3f7c1af731fe3cdbca9fa46bf/src/vs/base/node/zip.ts#L51
function modeFromEntry(entry) {
    const attr = entry.externalFileAttributes >> 16 || 33188

    return [448 /* S_IRWXU */, 56 /* S_IRWXG */, 7 /* S_IRWXO */]
        .map((mask) => attr & mask)
        .reduce((a, b) => a + b, attr & 61440 /* S_IFMT */)
}

yauzl.open(zipFilePath, { lazyEntries: true }, (err, zipfile) => {
    if (err) throw err

    zipfile.readEntry()
    zipfile.on("entry", (entry) => {
        if (!sourcePathRegex.test(entry.fileName)) {
            // Directory file names end with '/'
            // Note that entries for directories themselves are optional
            zipfile.readEntry()
        } else {
            // file entry
            console.log("Opening read stream: " + entry.fileName)
            const fileName = entry.fileName.replace(sourcePathRegex, "")
            const outputPath = path.join(destPath, extensionDestFolder, fileName)
            const outputDirectory = path.dirname(outputPath)
            console.log("Creating directory: " + outputDirectory)
            fse.mkdirpSync(outputDirectory)

            zipfile.openReadStream(entry, (err, readStream) => {
                if (err) throw err
                const mode = modeFromEntry(entry)
                const istream = fs.createWriteStream(outputPath, { mode })
                readStream.pipe(istream)
                readStream.on("end", () => {
                    zipfile.readEntry()
                })
                istream.once("error", (e) => {
                    console.error(e)
                })
                readStream.once("error", (e) => console.error(e))
            })
        }
    })
})

console.log("Installing extension...")
