// add-to-path.js
// This is a Node-side script to add Oni2 to their path.
// This is done in node, to get sudo-prompt.
const fs = require("fs");
const os = require("os");
const path = require("path");
const sudo = require("sudo-prompt");

const isWindows = () => os.platform() === "win32"
const isMac = () => os.platform() === "darwin"
const isLinux = () => os.platform() === "linux"

const getLinkPath = () => (isMac() || isLinux() ? "/usr/local/bin/oni2" : "")

const isAddedToPath = () => {
    try {
        fs.lstatSync(getLinkPath());
    } catch (_) {
        return false
    }
    return true
}
const removeFromPath = () => (isMac() || isLinux ? fs.unlinkSync(getLinkPath()) : false)

const addToPath = async () => {
    console.log("Starting...");

    if (!isAddedToPath() && !isWindows()) {
        const appDirectory = path.join(path.dirname(process.mainModule.filename), "..");
        const options = { name: "Oni2", icns: path.join(appDirectory, "assets", "images", "Onivim2.icns") };
        let linkPath = "";

        if (isMac()) {
            linkPath = "/Applications/Onivim2.app/Contents/MacOS/Oni2";
        } else {
            linkPath = ""; // TODO.
        }

        console.log(`The app directory is ${appDirectory}`);
        console.log(`The options are ${options.icns}`);
        console.log(`The link path is ${linkPath}`);
        await _runSudoCommand(`ln -fs ${linkPath} ${getLinkPath()}`, options);
    }

    console.log("Done!");
}

const _runSudoCommand = async (command, options) => {
    console.log("In Run....");
    return new Promise(resolve => {
        sudo.exec(command, options, (error, stdout, stderr) => {
            resolve({ error, stdout, stderr });
        });
    });
}

console.log("We are in node land...");
(async () => {
    try {
        await Promise.resolve(addToPath());
    } catch (_) {}
})();