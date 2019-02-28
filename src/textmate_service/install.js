const cp = require("child_process");
const fs = require("fs");
const path = require("path");

const rootDir = path.join(__dirname, "..", "..");
const vendorDir = path.join(rootDir, "vendor");

const nodeVendorDir = path.join(vendorDir, "node-v10.15.1");
const yarnScript = path.join(vendorDir, "yarn-v1.14.0", "yarn-1.14.0.js");

let nodeBinaryPath;

if (process.platform === "win32") {
    nodeBinaryPath = path.join(nodeVendorDir, "win-x64", "node.exe");
} else if (process.platform === "darwin") {
    nodeBinaryPath = path.join(nodeVendorDir, "osx", "node");
} else {
    nodeBinaryPath = path.join(nodeVendorDir, "linux-x64", "node")
}

cp.spawnSync(nodeBinaryPath, [yarnScript, "install"], {stdio: "inherit"});
