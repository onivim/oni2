const cp = require("child_process");
const fs = require("fs-extra");
const path = require("path");

let curBin = process.env["cur__bin"];
console.log("Bin folder: " + curBin);
console.log("Working directory: " + process.cwd());

const rootDirectory = process.cwd();
const releaseDirectory = path.join(process.cwd(), "_release");

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
};

if (process.platform == "darwin") {

  const executables = [
    "Oni2",
    "Oni2_editor",
  ];

  const appDirectory = path.join(releaseDirectory, "Onivim2.App");
  const contentsDirectory = path.join(appDirectory, "Contents");
  const binaryDirectory = path.join(contentsDirectory, "MacOS");
  const libsDirectory = path.join(contentsDirectory, "libs");
  const extensionsDestDirectory = path.join(contentsDirectory, "extensions");

  const plistFile = path.join(contentsDirectory, "Info.plist");

  const plistContents = {
      CFBundleName: "Onivim2",
      CFBundleDisplayName: "Onivim 2",
      CFBundleIdentifier: "com.outrunlabs.onivim2",
      CFBundleVersion: "0.01",
      CFBundlePackageType: "APPL",
      CFBundleSignature: "????",
      CFBundleExecutable: "Oni2"
  };

  fs.mkdirpSync(libsDirectory);
  fs.mkdirpSync(extensionsDestDirectory);

  fs.writeFileSync(plistFile, require("plist").build(plistContents));

  // Copy bins over
  copy(curBin, binaryDirectory);
  copy(extensionsSourceDirectory, extensionsDestDirectory);

  shell(`dylibbundler -b -x "${path.join(binaryDirectory, "Oni2_editor")}" -d "${libsDirectory}" -cd`);

  const dmgPath = path.join(releaseDirectory, "Onivim2.dmg");
  const basePath = releaseDirectory;

  const ee = require("appdmg")({
    target: dmgPath,
    basepath: basePath,
    specification: {
        title: "Onivim 2",
        background: path.join(rootDirectory, "assets", "images", "dmg-background.png");
    },
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
  });

  ee.on("progress", info => {
    if (info.type == "step-begin") {
        console.log("[dmg]: " +info.title);
    }
  });

  ee.on("finish", () => {
    console.log("[dmg] Success!");
  });

  ee.on("error", error => {
    console.error("[dmg] Failed with: " + error);
  });
   

} else {
  const platformReleaseDirectory = path.join(releaseDirectory, process.platform);
  const extensionsDestDirectory = path.join(releaseDirectory, "extensions");
  fs.mkdirpSync(platformReleaseDirectory);

  fs.copySync(curBin, binaryDirectory, { deference: true});
  fs.copySync(extensionsSourceDirectory, extensionsDestDirectory, {deference: true});
}
