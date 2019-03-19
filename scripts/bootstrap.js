const fs = require("fs");
const path = require("path");

const rootDir = path.join(__dirname, "..");
const configPath = path.join(rootDir, "assets", "configuration");
const vendorPath = path.join(rootDir, "vendor");

const outputFile = path.join(configPath, "setup.json");

const isMac = process.platform === "darwin";
const isWindows = process.platform === "win32";
const isLinux = !isMac && !isWindows;

let nodePath;
let textMateServicePath = path.join(rootDir, "src", "textmate_service", "lib", "src", "index.js");
let extensionsPath = path.join(rootDir, "extensions");
let neovimPath;
let configurationPath = path.join(configPath, "configuration.json");
let keybindingsPath = path.join(configPath, "keybindings.json");

const getCygwinPath = (inputPath) => { return inputPath.replace(/\\/g, "/") }

if (isWindows) {
    nodePath = getCygwinPath(path.join(vendorPath, "node-v10.15.1", "win-x64", "node.exe"));
    textMateServicePath = getCygwinPath(textMateServicePath);
    extensionsPath = getCygwinPath(extensionsPath);
    neovimPath = getCygwinPath(path.join(vendorPath, "neovim-0.3.3", "nvim-win64", "bin", "nvim.exe"));
    configurationPath = getCygwinPath(configurationPath);
    keybindingsPath = getCygwinPath(keybindingsPath);
} else if (isMac) {
    nodePath = path.join(vendorPath, "node-v10.15.1", "osx", "node")
    neovimPath = path.join(vendorPath, "neovim-0.3.3", "nvim-osx64", "bin", "nvim")
} else if (isLinux) {
    nodePath = path.join(vendorPath, "node-v10.15.1", "linux-x64", "node")
    neovimPath = path.join(vendorPath, "neovim-0.3.3", "nvim-linux64", "bin", "nvim")
} else {
    console.error("Unknown OS...aborting.");
    return;
}

const config = {
    neovim: neovimPath,
    node: nodePath,
    configuration: configurationPath,
    textmateService: textMateServicePath,
    bundledExtensions: extensionsPath,
    keybindings: keybindingsPath,
}
const oniConfig = JSON.stringify(config)

if (!fs.existsSync(configPath)) {
    fs.mkdirSync(configPath);

    if (!fs.existsSync(configPath)) {
        console.error(`Config folder ${configPath} can not be made!`);
        return;
    }
}

if (fs.existsSync(outputFile)) {
    fs.unlinkSync(outputFile)

    if (fs.existsSync(outputFile)) {
        console.error(`Output file ${outputFile} can not be deleted!`);
        return;
    }
}

console.log(`Writing to ${outputFile}...`)
fs.writeFileSync(outputFile, oniConfig);

console.log(`Done!`)
return 0