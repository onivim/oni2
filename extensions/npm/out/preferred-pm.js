"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.findPreferredPM = void 0;
const findWorkspaceRoot = require("../node_modules/find-yarn-workspace-root");
const findUp = require("find-up");
const path = require("path");
const whichPM = require("which-pm");
const vscode_1 = require("vscode");
async function pathExists(filePath) {
    try {
        await vscode_1.workspace.fs.stat(vscode_1.Uri.file(filePath));
    }
    catch (_a) {
        return false;
    }
    return true;
}
async function isPNPMPreferred(pkgPath) {
    if (await pathExists(path.join(pkgPath, 'pnpm-lock.yaml'))) {
        return true;
    }
    if (await pathExists(path.join(pkgPath, 'shrinkwrap.yaml'))) {
        return true;
    }
    if (await findUp('pnpm-lock.yaml', { cwd: pkgPath })) {
        return true;
    }
    return false;
}
async function isYarnPreferred(pkgPath) {
    if (await pathExists(path.join(pkgPath, 'yarn.lock'))) {
        return true;
    }
    try {
        if (typeof findWorkspaceRoot(pkgPath) === 'string') {
            return true;
        }
    }
    catch (err) { }
    return false;
}
const isNPMPreferred = (pkgPath) => {
    return pathExists(path.join(pkgPath, 'package-lock.json'));
};
async function findPreferredPM(pkgPath) {
    const detectedPackageManagers = [];
    if (await isNPMPreferred(pkgPath)) {
        detectedPackageManagers.push('npm');
    }
    if (await isYarnPreferred(pkgPath)) {
        detectedPackageManagers.push('yarn');
    }
    if (await isPNPMPreferred(pkgPath)) {
        detectedPackageManagers.push('pnpm');
    }
    const pmUsedForInstallation = await whichPM(pkgPath);
    if (pmUsedForInstallation && !detectedPackageManagers.includes(pmUsedForInstallation.name)) {
        detectedPackageManagers.push(pmUsedForInstallation.name);
    }
    const multiplePMDetected = detectedPackageManagers.length > 1;
    return {
        name: detectedPackageManagers[0] || 'npm',
        multiplePMDetected
    };
}
exports.findPreferredPM = findPreferredPM;
//# sourceMappingURL=preferred-pm.js.map