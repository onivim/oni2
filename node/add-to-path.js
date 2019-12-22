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
        const options = { name: "Oni2", icns: path.join(appDirectory, "assets", "images", "Onivim2.icns") }; // TODO: I need to check this in AppImage.
        let linkPath = "";

        if (isMac()) {
            linkPath = "/Applications/Onivim2.app/Contents/MacOS/Oni2";
        } else {
            linkPath = ""; // TODO.
        }

        await _runSudoCommand(`ln -fs ${linkPath} ${getLinkPath()}`, options);
    }

    console.log("Done!");
}

const _runSudoCommand = async (command, options) => {
    return new Promise(resolve => {
        sudo.exec(command, options, (error, stdout, stderr) => {
            resolve({ error, stdout, stderr });
        });
    });
}

(async () => {
    try {
        await Promise.resolve(addToPath());
    } catch (_) {}
})();