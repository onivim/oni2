const cp = require("child_process");
const fs = require("fs-extra");
const path = require("path");

let curBin = process.env["cur__bin"];
console.log("Bin folder: " + curBin);
console.log("Working directory: " + process.cwd());

const rootDirectory = process.cwd();
const releaseDirectory = path.join(process.cwd(), "_release");

const textmateServiceSourceDirectory = path.join(rootDirectory, "src", "textmate_service");
const extensionsSourceDirectory = path.join(process.cwd(), "extensions");
// const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions");

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
if (process.platform == "linux") {
  const result = cp.spawnSync("esy", ["scripts/linux/package-linux.sh"], { cwd: process.cwd(), env: process.env, stdio: 'inherit'});
  console.log(result.output.toString());
} else if (process.platform == "darwin") {
  const camomileRoot = shell("esy bash -c 'echo #{@opam/camomile.install}'").trim();
  const camomilePath = path.join(camomileRoot, "share", "camomile");

  const executables = [
    "Oni2",
    "Oni2_editor",
  ];

  const appDirectory = path.join(releaseDirectory, "Onivim2.App");
  const contentsDirectory = path.join(appDirectory, "Contents");
  const resourcesDirectory = path.join(contentsDirectory, "Resources");
  const binaryDirectory = path.join(contentsDirectory, "MacOS");
  const libsDirectory = path.join(contentsDirectory, "libs");
  const extensionsDestDirectory = path.join(contentsDirectory, "extensions");
  const textmateServiceDestDirectory = path.join(contentsDirectory, "textmate_service");

  const imageSourceDirectory = path.join(rootDirectory, "assets", "images");
  const iconSourcePath = path.join(imageSourceDirectory, "Onivim2.icns");

  const plistFile = path.join(contentsDirectory, "Info.plist");

  const plistContents = {
      CFBundleName: "Onivim2",
      CFBundleDisplayName: "Onivim 2",
      CFBundleIdentifier: "com.outrunlabs.onivim2",
      CFBundleIconFile: "Onivim2",
      CFBundleVersion: "0.01",
      CFBundlePackageType: "APPL",
      CFBundleSignature: "????",
      CFBundleExecutable: "Oni2"
  };

  fs.mkdirpSync(libsDirectory);
  fs.mkdirpSync(extensionsDestDirectory);
  fs.mkdirpSync(textmateServiceDestDirectory);
  fs.mkdirpSync(resourcesDirectory);

  fs.writeFileSync(plistFile, require("plist").build(plistContents));

  // Copy bins over
  copy(curBin, binaryDirectory);
  copy(extensionsSourceDirectory, contentsDirectory);
  copy(textmateServiceSourceDirectory, contentsDirectory);
  copy(camomilePath, contentsDirectory);
  copy(getRipgrepPath(), path.join(binaryDirectory, "rg"));
  copy(getNodePath(), path.join(binaryDirectory, "node"));

  // Copy icon
  copy(iconSourcePath, path.join(resourcesDirectory, "Onivim2.icns"));

  shell(`dylibbundler -b -x "${path.join(binaryDirectory, "Oni2_editor")}" -d "${libsDirectory}" -cd`);

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
  fs.removeSync(path.join(binaryDirectory, "setup.json"));
} else {
  const platformReleaseDirectory = path.join(releaseDirectory, process.platform);
  const extensionsDestDirectory = path.join(platformReleaseDirectory, "extensions");
  const textmateServiceDestDirectory = path.join(platformReleaseDirectory, "textmate_service");
  fs.mkdirpSync(platformReleaseDirectory);

  copy(getRipgrepPath(), path.join(platformReleaseDirectory, process.platform == "win32" ? "rg.exe" : "rg"));
  copy(getNodePath(), path.join(platformReleaseDirectory, process.platform == "win32" ? "node.exe" : "node"));
  fs.copySync(curBin, platformReleaseDirectory, { deference: true});
  fs.copySync(extensionsSourceDirectory, extensionsDestDirectory, {deference: true});
  fs.copySync(textmateServiceSourceDirectory, textmateServiceDestDirectory, {deference: true});
  fs.removeSync(path.join(platformReleaseDirectory, "setup.json"));
}
