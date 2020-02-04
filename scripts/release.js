const cp = require("child_process");
const fs = require("fs-extra");
const path = require("path");
const package = require("./../package.json")

let curBin = process.env["cur__bin"];
console.log("Bin folder: " + curBin);
console.log("Working directory: " + process.cwd());

const rootDirectory = process.cwd();
const vendorDirectory = path.join(rootDirectory, "vendor");
const releaseDirectory = path.join(rootDirectory, "_release");

// Delete releaseDirectory, and remake
fs.removeSync(releaseDirectory);
fs.mkdirpSync(releaseDirectory);

const nodeScriptSourceDirectory = path.join(process.cwd(), "node");
const extensionsSourceDirectory = path.join(process.cwd(), "extensions");
// const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions");

const eulaFile = path.join(process.cwd(), "Outrun-Labs-EULA-v1.1.md");
const thirdPartyFile = path.join(process.cwd(), "ThirdPartyLicenses.txt");

let camomileRoot = process.argv[2];
let camomilePath = path.join(camomileRoot, "share", "camomile");

console.log("Camomile path: " + camomilePath);

const copy = (source, dest) => {
    console.log(`Copying from ${source} to ${dest}`);
     if (process.platform == "darwin") {
        shell(`cp -r "${source}" "${dest}"`)
     } else {
        fs.copySync(source, dest);
     }
    console.log("Successfully copied.");
};

const shell = (cmd) => {
    console.log(`[shell] ${cmd}`);
    const out = cp.execSync(cmd);
    console.log(`[shell - output]: ${out.toString("utf8")}`);
    return out.toString("utf8");
};

const getRipgrepPath = () => {
    const rg = "ripgrep-v0.10.0";

    if (process.platform == "darwin") {
        return path.join(rootDirectory, "vendor", rg, "mac", "rg");
    } else if (process.platform == "win32") {
        return path.join(rootDirectory, "vendor", rg, "windows", "rg.exe");
    } else {
        return path.join(rootDirectory, "vendor", rg, "linux", "rg");
    }
}

const getNodePath = () => {
    const nodeDir = "node-v10.15.1";

    if (process.platform == "darwin") {
        return path.join(rootDirectory, "vendor", nodeDir, "osx", "node");
    } else if (process.platform == "win32") {
        return path.join(rootDirectory, "vendor", nodeDir, "win-x64", "node.exe");
    } else {
        return path.join(rootDirectory, "vendor", nodeDir, "linux-x64", "node");
    }
}

const getRlsPath = () => {
    const rlsDir = "reason-language-server";

    if (process.platform == "darwin") {
        return path.join(vendorDirectory, rlsDir, "bin.native")
    } else if (process.platform == "win32") {
        return path.join(vendorDirectory, rlsDir, "bin.native.exe");
    } else {
        return path.join(vendorDirectory, rlsDir, "bin.native.linux");
    }
}

const updateIcon = (rcedit, exe, iconFile) => {
    console.log(`Updating ${exe} icon`)

    rcedit(exe, {
        icon: iconFile
    })

    console.log(`Successfully updated icon.`)
}

if (process.platform == "linux") {
  const result = cp.spawnSync("esy", ["scripts/linux/package-linux.sh"], { cwd: process.cwd(), env: process.env, stdio: 'inherit'});
  console.log(result.output.toString());
} else if (process.platform == "darwin") {
  const executables = [
    "Oni2",
    "Oni2_editor",
    "rg",
    "rls",
    "node"
  ];

  const appDirectory = path.join(releaseDirectory, "Onivim2.app");
  const contentsDirectory = path.join(appDirectory, "Contents");
  const resourcesDirectory = path.join(contentsDirectory, "Resources");
  const binaryDirectory = path.join(contentsDirectory, "MacOS");
  const frameworksDirectory = path.join(contentsDirectory, "Frameworks");

  const imageSourceDirectory = path.join(rootDirectory, "assets", "images");
  const iconSourcePath = path.join(imageSourceDirectory, "Onivim2.icns");

  const plistFile = path.join(contentsDirectory, "Info.plist");

  const plistContents = {
      CFBundleName: "Onivim2",
      CFBundleDisplayName: "Onivim 2",
      CFBundleIdentifier: "com.outrunlabs.onivim2",
      CFBundleIconFile: "Onivim2",
      CFBundleVersion: `${package.version}`,
      CFBundlePackageType: "APPL",
      CFBundleSignature: "????",
      CFBundleExecutable: "Oni2",
      NSHighResolutionCapable: true,
  };

  fs.mkdirpSync(frameworksDirectory);
  fs.mkdirpSync(resourcesDirectory);
  fs.writeFileSync(plistFile, require("plist").build(plistContents));

  // Copy bins over
  copy(curBin, binaryDirectory);

  // Copy run helper script over
  copy("scripts/osx/run.sh", "_release/run.sh");

  copy(extensionsSourceDirectory, resourcesDirectory);
  copy(nodeScriptSourceDirectory, resourcesDirectory);
  copy(camomilePath, resourcesDirectory);
  copy(getRipgrepPath(), path.join(binaryDirectory, "rg"));
  copy(getNodePath(), path.join(binaryDirectory, "node"));
  copy(getRlsPath(), path.join(binaryDirectory, "rls"));

  // Remove setup.json prior to remapping bundled files,
  // so it doesn't get symlinked.
  fs.removeSync(path.join(binaryDirectory, "setup.json"));

  // We need to remap the binary files - we end up with font files, images, and configuration files in the bin folder
  // These should be in 'Resources' instead. Move everything that is _not_ a binary out, and symlink back in.
  const filesToBeMoved = fs.readdirSync(binaryDirectory).filter((f) => {
    console.log("- Checking executable: " + f);
    return executables.indexOf(f) == -1;
  });

  filesToBeMoved.forEach((file) => {
    const fileSrc = path.join(binaryDirectory, file);
    const fileDest = path.join(resourcesDirectory, file);
    console.log(`Moving file from ${fileSrc} to ${fileDest}.`);
    fs.moveSync(fileSrc, fileDest);
    const symlinkDest = path.join("../Resources", file);
    console.log(`Symlinking ${symlinkDest} -> ${fileSrc}`);
    fs.ensureSymlink(symlinkDest, fileSrc);
  });

  fs.copySync(eulaFile, path.join(resourcesDirectory, "EULA.md"));
  fs.copySync(thirdPartyFile, path.join(resourcesDirectory, "ThirdPartyLicenses.txt"));
  fs.copySync("scripts/osx/run.sh", path.join(resourcesDirectory, "run.sh"));
  fs.copySync("assets/images/Onivim2.icns", path.join(resourcesDirectory, "Onivim2.icns"));

  // Copy icon
  copy(iconSourcePath, path.join(resourcesDirectory, "Onivim2.icns"));

  shell(`dylibbundler -b -x "${path.join(binaryDirectory, "Oni2_editor")}" -d "${frameworksDirectory}" -p "@executable_path/../Frameworks/" -cd`);

  const entitlementsPath = path.join(releaseDirectory, "entitlements.plist");
  const entitlementsContents = {
      "com.apple.security.cs.allow-jit": true,
      "com.apple.security.cs.allow-unsigned-executable-memory": true,
      "com.apple.security.cs.disable-library-validation": true,
  };
  fs.writeFileSync(entitlementsPath, require("plist").build(entitlementsContents));

  const dmgPath = path.join(releaseDirectory, "Onivim2.dmg");
  const dmgJsonPath = path.join(releaseDirectory, "appdmg.json");
  const basePath = releaseDirectory;

  const dmgJson = {
    title: "Onivim 2",
    background: path.join(imageSourceDirectory, "dmg-background.png"),
    format: "ULFO",
    window: {
        size: {
            width: 660,
            height: 400,
        }
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
            path: "/Applications"
        }
    ]
  };
  fs.writeFileSync(dmgJsonPath, JSON.stringify(dmgJson));
} else {
  const platformReleaseDirectory = path.join(releaseDirectory, process.platform);
  const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions");
  const nodeScriptDestDirectory = path.join(platformReleaseDirectory, "node");
  fs.mkdirpSync(platformReleaseDirectory);

  copy(getRipgrepPath(), path.join(platformReleaseDirectory, process.platform == "win32" ? "rg.exe" : "rg"));
  copy(getNodePath(), path.join(platformReleaseDirectory, process.platform == "win32" ? "node.exe" : "node"));
  copy(getRlsPath(), path.join(platformReleaseDirectory, process.platform == "win32" ? "rls.exe" : "rls"));
  copy(camomilePath, path.join(platformReleaseDirectory, "camomile"));
  const imageSourceDirectory = path.join(rootDirectory, "assets", "images");
  const iconFile = path.join(imageSourceDirectory, "oni2.ico");
  fs.copySync(iconFile, path.join(platformReleaseDirectory, "oni2.ico"));
  fs.copySync(eulaFile, path.join(platformReleaseDirectory, "EULA.md"));
  fs.copySync(thirdPartyFile, path.join(platformReleaseDirectory, "ThirdPartyLicenses.txt"));
  fs.copySync(curBin, platformReleaseDirectory, { deference: true});
  fs.copySync(extensionsSourceDirectory, extensionsDestDirectory, {deference: true});
  fs.copySync(nodeScriptSourceDirectory, nodeScriptDestDirectory, {deference: true});
  fs.removeSync(path.join(platformReleaseDirectory, "setup.json"));

  // Now that we've copied set the app icon up correctly.
  // rcedit is imported here as its a big import, only needed for windows.
  // The image path here does not need to be in the release dir, as its just to do the embed.
  const rcedit = require("rcedit");
  updateIcon(rcedit, path.join(platformReleaseDirectory, "Oni2.exe"), iconFile);
  updateIcon(rcedit, path.join(platformReleaseDirectory, "Oni2_editor.exe"), iconFile);
}
