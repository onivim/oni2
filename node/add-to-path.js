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

const getOptions = () => {
    let appDirectory = "";

    if (isMac()) {
        // Valid for an install from a dmg.
        appDirectory = path.join(path.dirname(process.mainModule.filename), "..");
    } else {
        // Valid path for an AppImage or Linux tar.gz
        appDirectory = path.join(path.dirname(process.mainModule.filename), "..", "..");
    }

    // Valid for an install from a dmg.
    let imgPath = path.join(appDirectory, "Onivim2.icns");

    if (!fs.existsSync(imgPath)) {
        // Valid path when in a development build.
        imgPath = path.join(appDirectory, "assets", "images", "Onivim2.icns");
    }

    if (!fs.existsSync(imgPath)) {
        // Valid path for an AppImage or Linux tar.gz
        imgPath = path.join(appDirectory, "..", "Onivim2.icns");
    }

    const options = { name: "Oni2", icns: imgPath };

    return options;
}

const removeFromPath = async () => {
    if (isAddedToPath() && !isWindows()) {
        await runSudoCommand(`rm ${getLinkPath()}`, getOptions());
    }
}

const addToPath = async () => {
    if (!isAddedToPath() && !isWindows()) {

        let linkDest = "";

        if (isMac()) {
            // Valid for an install from a dmg.
            linkDest = path.join(appDirectory, "run.sh");
        } else if (process.env.APPIMAGE) {
            // Valid path for an AppImage.
            linkDest = process.env.APPIMAGE;
        } else {
            // Valid path for Linux tar.gz
            linkDest = path.join(appDirectory, "..", "AppRun");
        }

        if (!fs.existsSync(linkDest)) {
            return
        }

        await runSudoCommand(`ln -fs ${linkDest} ${getLinkPath()}`, getOptions());
    }
}

const runSudoCommand = async (command, options) => {
    return new Promise(resolve => {
        sudo.exec(command, options, (error, stdout, stderr) => {
            resolve({ error, stdout, stderr });
        });
    });
}

const toggleAddToPath = async () => {
    if (isAddedToPath()) {
        await removeFromPath()
    } else {
        await addToPath()
    }
}

(async () => {
    try {
        await Promise.resolve(toggleAddToPath());
    } catch (_) {}
})();
