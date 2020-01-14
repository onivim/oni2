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
const removeFromPath = () => {
    if (isAddedToPath() && !isWindows()) {
        fs.unlinkSync(getLinkPath())
    }
}

const addToPath = async () => {
    if (!isAddedToPath() && !isWindows()) {

        let appDirectory = "";

        if (isMac()) {
            // Valid for an install from a dmg.
            appDirectory = path.join(path.dirname(process.mainModule.filename), "..");
        } else {
            // Valid path for an AppImage or Linux tar.gz
            appDirectory = path.join(path.dirname(process.mainModule.filename), "..", "..");
        }

        console.log(`The appDirectory is apparently : ${appDirectory}`);

        // Valid for an install from a dmg.
        let imgPath = path.join(appDirectory, "Onivim2.icns");

        if (!fs.existsSync(imgPath)) {
            // Valid path when in a development build.
            imgPath = path.join(appDirectory, "assets", "images", "Onivim2.icns");
        }

        if (!fs.existsSync(imgPath)) {
            // Valid path for an AppImage or Linux tar.gz
            imgPath = path.join(appDirectory, "..", "Onivim2.png"); // TODO: Should be an icon file.
        }

        console.log(`The imgPath is apparently : ${imgPath}`);

        const options = { name: "Oni2", icns: imgPath };
        let linkDest = "";

        if (isMac()) {
            // Valid for an install from a dmg.
            linkDest = path.join(appDirectory, "run.sh");
        } else {
            // Valid path for an AppImage or Linux tar.gz
            linkDest = path.join(appDirectory, "..", "AppRun"); // TODO: This currently points to a /tmp/ location of the virtual file system. We actually want to point to the real AppImage in the AppImage case.
        }

        if (!fs.existsSync(linkDest)) {
            return
        }

        await runSudoCommand(`ln -fs ${linkDest} ${getLinkPath()}`, options);
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
        removeFromPath()
    } else {
        await addToPath()
    }
}

(async () => {
    try {
        await Promise.resolve(toggleAddToPath());
    } catch (_) {}
})();
