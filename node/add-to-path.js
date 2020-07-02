// add-to-path.js
// This is a Node-side script to add Oni2 to their path.
// This is done in node, to get sudo-prompt.
const fs = require("fs")
const os = require("os")
const path = require("path")
const sudo = require("sudo-prompt")

const isWindows = () => os.platform() === "win32"
const isMac = () => os.platform() === "darwin"
const isLinux = () => os.platform() === "linux"

const getLinkPath = () => (isMac() || isLinux() ? "/usr/local/bin/oni2" : "")

const isAddedToPath = () => {
    try {
        fs.lstatSync(getLinkPath())
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
        const appDirectory = path.join(path.dirname(process.mainModule.filename), "..")
        let imgPath = path.join(appDirectory, "Onivim2.icns")

        // TODO: Check this is valid for all use cases.
        if (!fs.existsSync(imgPath)) {
            imgPath = path.join(appDirectory, "assets", "images", "Onivim2.icns")
        }

        const options = { name: "Oni2", icns: imgPath }
        let linkDest = ""

        if (isMac()) {
            linkDest = path.join(appDirectory, "run.sh")
        } else {
            linkDest = "" // TODO.
        }

        if (!fs.existsSync(linkDest)) {
            return
        }

        await runSudoCommand(`ln -fs ${linkDest} ${getLinkPath()}`, options)
    }
}

const runSudoCommand = async (command, options) => {
    return new Promise((resolve) => {
        sudo.exec(command, options, (error, stdout, stderr) => {
            resolve({ error, stdout, stderr })
        })
    })
}

const toggleAddToPath = async () => {
    if (isAddedToPath()) {
        removeFromPath()
    } else {
        await addToPath()
    }
}

;(async () => {
    try {
        await Promise.resolve(toggleAddToPath())
    } catch (_) {}
})()
