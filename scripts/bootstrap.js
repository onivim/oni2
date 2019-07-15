const cp = require("child_process");
const fs = require("fs")
const path = require("path")

const rootDir = path.join(__dirname, "..")
const configPath = path.join(rootDir, "assets", "configuration")
const vendorPath = path.join(rootDir, "vendor")

const outputFile = path.join(configPath, "setup.json")

const isMac = process.platform === "darwin"
const isWindows = process.platform === "win32"
const isLinux = !isMac && !isWindows

let nodePath
let textMateServicePath = path.join(rootDir, "src", "textmate_service", "lib", "src", "index.js")
let extensionHostPath = path.join(rootDir, "src", "textmate_service", "node_modules", "vscode-exthost", "out", "bootstrap-fork.js");
let extensionsPath = path.join(rootDir, "extensions")
let developmentExtensionsPath = path.join(rootDir, "src", "development_extensions");
let rgPath = path.join(vendorPath, "ripgrep-v0.10.0")

let camomilePath = cp.execSync("esy bash -c \"echo #{@opam/camomile.install}\"").toString("utf8").trim();

const getCygwinPath = inputPath => {
    return inputPath.replace(/\\/g, "/")
}

if (isWindows) {
    nodePath = getCygwinPath(path.join(vendorPath, "node-v10.15.1", "win-x64", "node.exe"))
    textMateServicePath = getCygwinPath(textMateServicePath)
    extensionHostPath = getCygwinPath(extensionHostPath);
    extensionsPath = getCygwinPath(extensionsPath)
    developmentExtensionsPath = getCygwinPath(developmentExtensionsPath);
    rgPath = getCygwinPath(path.join(rgPath, "windows", "rg.exe"))
} else if (isMac) {
    nodePath = path.join(vendorPath, "node-v10.15.1", "osx", "node")
    rgPath = path.join(rgPath, "mac", "rg")
} else if (isLinux) {
    nodePath = path.join(vendorPath, "node-v10.15.1", "linux-x64", "node")
    rgPath = path.join(rgPath, "linux", "rg")
} else {
    console.error("Unknown OS...aborting.")
    return 1
}

const config = {
    node: nodePath,
    camomile: camomilePath,
    textmateService: textMateServicePath,
    bundledExtensions: extensionsPath,
    developmentExtensions: developmentExtensionsPath,
    extensionHost: extensionHostPath,
    rg: rgPath,
}
const oniConfig = JSON.stringify(config)

if (!fs.existsSync(configPath)) {
    fs.mkdirSync(configPath)

    if (!fs.existsSync(configPath)) {
        console.error(`Config folder ${configPath} can not be made!`)
        return 1
    }
}

if (fs.existsSync(outputFile)) {
    fs.unlinkSync(outputFile)

    if (fs.existsSync(outputFile)) {
        console.error(`Output file ${outputFile} can not be deleted!`)
        return 1
    }
}

console.log(`Writing to ${outputFile}...`)
fs.writeFileSync(outputFile, oniConfig)

console.log(`Done!`)
return 0
