const cp = require("child_process")
const fs = require("fs-extra")
const path = require("path")
const package = require("./../package.json")

let curBin = process.env["cur__bin"]
console.log("Bin folder: " + curBin)
console.log("Working directory: " + process.cwd())

let scriptArgs = process.argv.slice(2)
console.log("Arguments: " + scriptArgs)

let codesign = scriptArgs.indexOf("--codesign") != -1

const rootDirectory = process.cwd()
const vendorDirectory = path.join(rootDirectory, "vendor")
const releaseDirectory = path.join(rootDirectory, "_release")

// Delete releaseDirectory, and remake
fs.removeSync(releaseDirectory)
fs.mkdirpSync(releaseDirectory)

const nodeScriptSourceDirectory = path.join(process.cwd(), "node")
const extensionsSourceDirectory = path.join(process.cwd(), "extensions")
// const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions");

const eulaFile = path.join(process.cwd(), "Outrun-Labs-EULA-v1.1.md")
const thirdPartyFile = path.join(process.cwd(), "ThirdPartyLicenses.txt")
const sparkleFramework = path.join(rootDirectory, "vendor", "Sparkle-1.23.0", "Sparkle.framework")
const winSparkleDLL = path.join(
    rootDirectory,
    "vendor",
    "WinSparkle-0.7.0",
    "x64",
    "Release",
    "WinSparkle.dll",
)

const copy = (source, dest) => {
    console.log(`Copying from ${source} to ${dest}`)
    if (process.platform == "darwin") {
        shell(`cp -r "${source}" "${dest}"`)
    } else {
        fs.copySync(source, dest)
    }
    console.log("Successfully copied.")
}

const shell = (cmd) => {
    console.log(`[shell] ${cmd}`)
    const out = cp.execSync(cmd)
    console.log(`[shell - output]: ${out.toString("utf8")}`)
    return out.toString("utf8")
}

const winShell = (cmd) => {
    let oldEnv = process.env
    process.env = {
        PATH: process.env.PATH,
    }
    const res = shell(cmd)
    process.env = oldEnv
    return res
}

const getRipgrepPath = () => {
    const rg = "ripgrep-v0.10.0"

    if (process.platform == "darwin") {
        return path.join(rootDirectory, "vendor", rg, "mac", "rg")
    } else if (process.platform == "win32") {
        return path.join(rootDirectory, "vendor", rg, "windows", "rg.exe")
    } else {
        return path.join(rootDirectory, "vendor", rg, "linux", "rg")
    }
}

const getNodePath = () => {
    const nodeDir = "node-v12.17.0"

    if (process.platform == "darwin") {
        return path.join(rootDirectory, "vendor", nodeDir, "osx", "node")
    } else if (process.platform == "win32") {
        return path.join(rootDirectory, "vendor", nodeDir, "win-x64", "node.exe")
    } else {
        return path.join(rootDirectory, "vendor", nodeDir, "linux-x64", "node")
    }
}

const updateIcon = (rcedit, exe, iconFile) => {
    console.log(`Updating ${exe} icon`)

    // HACK: `rcedit` crashes with an assertion failure on windows:
    // Assertion failed: len < MAX_ENV_VAR_LENGTH, file c:\ws\deps\uv\src\win\util.c, line 1476
    let oldEnv = process.env
    process.env = {
        PATH: process.env.PATH,
    }
    fs.chmodSync(exe, 0755)
    rcedit(exe, {
        icon: iconFile,
    })
    process.env = oldEnv

    console.log(`Successfully updated icon.`)
}

if (process.platform == "linux") {
    const result = cp.spawnSync("esy", ["bash", "-c", "scripts/linux/package-linux.sh"], {
        cwd: process.cwd(),
        env: process.env,
        stdio: "inherit",
    })
    console.log(result.output.toString())
} else if (process.platform == "darwin") {
    const executables = ["Oni2", "Oni2_editor", "rg", "node"]

    const appDirectory = path.join(releaseDirectory, "Onivim2.app")
    const contentsDirectory = path.join(appDirectory, "Contents")
    const resourcesDirectory = path.join(contentsDirectory, "Resources")
    const binaryDirectory = path.join(contentsDirectory, "MacOS")
    const frameworksDirectory = path.join(contentsDirectory, "Frameworks")

    const imageSourceDirectory = path.join(rootDirectory, "assets", "images")
    const iconSourcePath = path.join(imageSourceDirectory, "Onivim2.icns")
    const documentIconSourcePath = path.join(imageSourceDirectory, "macDocumentIcons")

    const plistFile = path.join(contentsDirectory, "Info.plist")

    const numCommits = shell("git rev-list --count origin/master").replace(/(\r\n|\n|\r)/gm, "")
    const semvers = package.version.split(".")
    const bundleVersion = `${semvers[0]}.${semvers[1]}.${numCommits}`

    const plistContents = {
        CFBundleName: "Onivim 2",
        CFBundleDisplayName: "Onivim 2",
        CFBundleIdentifier: "com.outrunlabs.onivim2",
        CFBundleIconFile: "Onivim2",
        CFBundleVersion: bundleVersion,
        CFBundleShortVersionString: `${package.version}`,
        CFBundlePackageType: "APPL",
        CFBundleSignature: "????",
        CFBundleExecutable: "Oni2_editor",
        NSHighResolutionCapable: true,
        NSSupportsAutomaticGraphicsSwitching: true,
        CFBundleDocumentTypes: package.build.fileAssociations.map((fileAssoc) => {
            return {
                CFBundleTypeExtensions: fileAssoc.ext.map((ext) => ext.substr(1)),
                CFBundleTypeName: fileAssoc.name,
                CFBundleTypeRole: fileAssoc.role,
                CFBundleTypeOSTypes: ["TEXT", "utxt", "TUTX", "****"],
                CFBundleTypeIconFile: "macDocumentIcons/" + fileAssoc.icon.mac,
            }
        }),
        LSEnvironment: {
            ONI2_BUNDLED: "1",
            ONI2_LAUNCHED_FROM_FINDER: "1",
        },
        SUFeedURL: process.env.ONI2_APPCAST_BASEURL,
        NSAppTransportSecurity: {
            NSAllowsLocalNetworking: true,
        },
    }

    fs.mkdirpSync(frameworksDirectory)
    fs.mkdirpSync(resourcesDirectory)
    fs.writeFileSync(plistFile, require("plist").build(plistContents))

    // Copy bins over
    copy(curBin, binaryDirectory)

    // Copy run helper script over
    copy("scripts/osx/run.sh", "_release/run.sh")

    copy(extensionsSourceDirectory, resourcesDirectory)
    copy(nodeScriptSourceDirectory, resourcesDirectory)
    copy(documentIconSourcePath, resourcesDirectory)
    copy(getRipgrepPath(), path.join(binaryDirectory, "rg"))
    copy(getNodePath(), path.join(binaryDirectory, "node"))

    // Folders to delete
    // TODO: Move this into our VSCode packaging, there are a lot of files we don't need to bundle at all
    let resourceFoldersToDelete = [
        "node/node_modules/vscode-exthost/out/vs/workbench/services/search/test",
    ]

    resourceFoldersToDelete.forEach((p) => {
        let deletePath = path.join(resourcesDirectory, p)
        console.log("Deleting path: " + deletePath)
        fs.removeSync(deletePath)
    })

    // Remove setup.json prior to remapping bundled files,
    // so it doesn't get symlinked.
    fs.removeSync(path.join(binaryDirectory, "setup.json"))
    // Remove development plist file
    fs.removeSync(path.join(binaryDirectory, "Info.plist"))

    // We need to remap the binary files - we end up with font files, images, and configuration files in the bin folder
    // These should be in 'Resources' instead. Move everything that is _not_ a binary out, and symlink back in.
    const filesToBeMoved = fs.readdirSync(binaryDirectory).filter((f) => {
        console.log("- Checking executable: " + f)
        return executables.indexOf(f) == -1
    })

    filesToBeMoved.forEach((file) => {
        const fileSrc = path.join(binaryDirectory, file)
        const fileDest = path.join(resourcesDirectory, file)
        console.log(`Moving file from ${fileSrc} to ${fileDest}.`)
        fs.moveSync(fileSrc, fileDest)
        const symlinkDest = path.join("../Resources", file)
        console.log(`Symlinking ${symlinkDest} -> ${fileSrc}`)
        fs.ensureSymlink(symlinkDest, fileSrc)
    })

    fs.copySync(eulaFile, path.join(resourcesDirectory, "EULA.md"))
    fs.copySync(thirdPartyFile, path.join(resourcesDirectory, "ThirdPartyLicenses.txt"))
    fs.copySync("scripts/osx/run.sh", path.join(resourcesDirectory, "run.sh"))
    fs.copySync("assets/images/Onivim2.icns", path.join(resourcesDirectory, "Onivim2.icns"))

    // Copy icon
    copy(iconSourcePath, path.join(resourcesDirectory, "Onivim2.icns"))

    // fs.copySync(sparkleFramework, path.join(frameworksDirectory, "Sparkle.framework"));

    shell(`cp -R "${sparkleFramework}" "${path.join(frameworksDirectory, "Sparkle.framework")}"`)

    shell(
        `dylibbundler -b -x "${path.join(
            binaryDirectory,
            "Oni2_editor",
        )}" -d "${frameworksDirectory}" -p "@executable_path/../Frameworks/" -cd`,
    )

    const frameworks = fs.readdirSync(frameworksDirectory)

    // Make sure these are codesigned as well in codesign.sh
    // Must be kept in sync with:
    // src/scripts/osx/codesign.sh
    const frameworksWhiteList = ["Sparkle.framework"]

    const disallowedFrameworks = frameworks.filter(
        (framework) =>
            !frameworksWhiteList.some(
                (allowedFramework) => framework.indexOf(allowedFramework) >= 0,
            ),
    )

    if (disallowedFrameworks.length > 0) {
        console.error("Found a dynamic library: " + JSON.stringify(disallowedFrameworks))
        console.error("There should be only static libraries to successfully package.")
        throw "FrameworkFound"
    }

    const entitlementsPath = path.join(releaseDirectory, "entitlements.plist")
    const entitlementsContents = {
        "com.apple.security.cs.allow-jit": true,
        "com.apple.security.cs.allow-unsigned-executable-memory": true,
        "com.apple.security.cs.disable-library-validation": true,
        // Allow dyld environment variables. Needed because Onivim 2 uses
        //         dyld variables (such as @executable_path) to load libaries from
        //         within the .app bundle.
        // See: https://github.com/onivim/oni2/issues/1397
        "com.apple.security.cs.allow-dyld-environment-variables": true,
    }
    fs.writeFileSync(entitlementsPath, require("plist").build(entitlementsContents))

    if (codesign) {
        const cert = fs.readFileSync("./scripts/osx/test-certificate.p12", { encoding: "base64" })

        shell(
            `OSX_P12_CERTIFICATE="${cert}" CODESIGN_PASSWORD="OutrunLabs" CERTIFICATE_NAME="Oni2" ./scripts/osx/codesign.sh`,
        )
    }

    const dmgPath = path.join(releaseDirectory, "Onivim2.dmg")
    const dmgJsonPath = path.join(releaseDirectory, "appdmg.json")
    const basePath = releaseDirectory

    const dmgJson = {
        title: "Onivim 2",
        background: path.join(imageSourceDirectory, "dmg-background.png"),
        icon: path.join(imageSourceDirectory, "dmg-icon.icns"),
        format: "ULFO",
        window: {
            size: {
                width: 660,
                height: 400,
            },
        },
        contents: [
            {
                x: 180,
                y: 170,
                type: "file",
                path: appDirectory,
            },
            {
                x: 480,
                y: 170,
                type: "link",
                path: "/Applications",
            },
        ],
    }
    fs.writeFileSync(dmgJsonPath, JSON.stringify(dmgJson))
} else {
    const platformReleaseDirectory = path.join(releaseDirectory, process.platform)
    const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions")
    const nodeScriptDestDirectory = path.join(platformReleaseDirectory, "node")
    fs.mkdirpSync(platformReleaseDirectory)

    copy(
        getRipgrepPath(),
        path.join(platformReleaseDirectory, process.platform == "win32" ? "rg.exe" : "rg"),
    )
    copy(
        getNodePath(),
        path.join(platformReleaseDirectory, process.platform == "win32" ? "node.exe" : "node"),
    )
    if (process.platform == "win32") {
        const numCommits = winShell(
            '"C:\\Program Files\\Git\\cmd\\git.exe" rev-list --count origin/master',
        ).replace(/(\r\n|\n|\r)/gm, "")
        const semvers = package.version.split(".")
        const bundleVersion = `${semvers[0]}.${semvers[1]}.${numCommits}`

        const oni2Ini = `
        [Application]
        Version = ${bundleVersion}
        `
        fs.writeFileSync(path.join(platformReleaseDirectory, "Oni2.ini"), oni2Ini)
        copy(winSparkleDLL, path.join(platformReleaseDirectory, "WinSparkle.dll"))
    }
    const imageSourceDirectory = path.join(rootDirectory, "assets", "images")
    const iconFile = path.join(imageSourceDirectory, "oni2.ico")
    fs.copySync(iconFile, path.join(platformReleaseDirectory, "oni2.ico"))
    fs.copySync(eulaFile, path.join(platformReleaseDirectory, "EULA.md"))
    fs.copySync(thirdPartyFile, path.join(platformReleaseDirectory, "ThirdPartyLicenses.txt"))
    fs.copySync(curBin, platformReleaseDirectory, { dereference: true })
    fs.copySync(extensionsSourceDirectory, extensionsDestDirectory, { dereference: true })
    fs.copySync(nodeScriptSourceDirectory, nodeScriptDestDirectory, { dereference: true })
    fs.removeSync(path.join(platformReleaseDirectory, "setup.json"))

    // Now that we've copied set the app icon up correctly.
    // rcedit is imported here as its a big import, only needed for windows.
    // The image path here does not need to be in the release dir, as its just to do the embed.
    const rcedit = require("rcedit")
    updateIcon(rcedit, path.join(platformReleaseDirectory, "Oni2.exe"), iconFile)
    updateIcon(rcedit, path.join(platformReleaseDirectory, "Oni2_editor.exe"), iconFile)
}
