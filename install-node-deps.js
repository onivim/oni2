const cp = require("child_process")
const fs = require("fs")
const path = require("path")

const rootDir = __dirname
const vendorDir = path.join(rootDir, "vendor")

const nodeVendorDir = path.join(vendorDir, "node-v12.17.0")
const yarnScript = path.join(vendorDir, "yarn-v1.14.0", "yarn-1.14.0.js")

let nodeBinaryPath

if (process.platform === "win32") {
    nodeBinaryPath = path.join(nodeVendorDir, "win-x64", "node.exe")
} else if (process.platform === "darwin") {
    nodeBinaryPath = path.join(nodeVendorDir, "osx", "node")
} else {
    nodeBinaryPath = path.join(nodeVendorDir, "linux-x64", "node")
}

let useProductionDeps = process.argv.filter((i) => i.indexOf("-prod") >= 0).length > 0

console.log(`-- Production: ${useProductionDeps}`)

let args = [yarnScript, "install", "--flat", "--production=true"]

const packagesToInstall = ["node", "extensions"]

packagesToInstall.forEach((pkg) => {
    const fullPath = path.join(rootDir, pkg)
    console.log("Installing packages at: " + fullPath)
    console.log("Running: " + nodeBinaryPath + " | " + JSON.stringify(args))
    const result = cp.spawnSync(nodeBinaryPath, args, { stdio: "inherit", cwd: fullPath })
    if (result.status != 0) {
        console.error(`Process exited with code ${result.status}`)
        console.error(`stderr: ${result.stderr}`)
        throw result.error
    }
})

console.log("Node dependencies successfully installed.")
