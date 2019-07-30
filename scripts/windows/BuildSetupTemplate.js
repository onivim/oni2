// BuildSetupTemplate.js
//
// Helper script to insert template variables into the setup template

const path = require("path")
const fs = require("fs")
const os = require("os")

const _ = require("lodash")
const shelljs = require("shelljs")

const sourceFile = path.join(__dirname, "setup.template.iss")
const destFile = path.join(__dirname, "..", "..", "_release", "setup.iss")

shelljs.rm(destFile)

shelljs.cp(sourceFile, destFile)

const packageJsonContents = fs.readFileSync(path.join(__dirname, "..", "..", "package.json"))
const packageMeta = JSON.parse(packageJsonContents)
const { version, name } = packageMeta
const prodName = packageMeta.build.productName
const executableName = `Oni2.exe`
const pathVariable = "{app}"

let buildFolderPrefix = os.arch() === "x32" ? "ia32-" : ""

if (process.env["APPVEYOR"]) {
    buildFolderPrefix = process.env["PLATFORM"] === "x86" ? "ia32-" : ""
}

// Replace template variables

const valuesToReplace = {
    AppName: prodName,
    AppExecutableName: executableName,
    AppSetupExecutableName: `${prodName}-${version}-${buildFolderPrefix}win`,
    Version: version,
    SetupIconFile: path.join(__dirname, "..", "..", "assets", "images", "oni2.ico"),
    SourcePath: path.join(__dirname, "..", "..", "_release", `win32`, "*"),
    WizardImageFilePath: path.join(__dirname, "setup", "Onivim2_128.bmp"),
    WizardSmallImageFilePath: path.join(__dirname, "setup", "Onivim2_54.bmp"),
    cliPath: pathVariable,
}

const addToEnv = `
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};${pathVariable}"; Tasks: addtopath; Check: NeedsAddPath(ExpandConstant('${pathVariable}'))

Root: HKCU; Subkey: "SOFTWARE\\Classes\\*\\shell\\${prodName}"; ValueType: expandsz; ValueName: ""; ValueData: "Open with ${prodName}"; Tasks: addToRightClickMenu; Flags: uninsdeletekey
Root: HKCU; Subkey: "SOFTWARE\\Classes\\*\\shell\\${prodName}"; ValueType: expandsz; ValueName: "Icon"; ValueData: "{app}\\oni2.ico"; Tasks: addToRightClickMenu
Root: HKCU; Subkey: "SOFTWARE\\Classes\\*\\shell\\${prodName}\\command"; ValueType: expandsz; ValueName: ""; ValueData: """{app}\\${executableName}"" ""%1"""; Tasks: addToRightClickMenu
`

function getFileRegKey(ext, desc) {

    // Check that an actual extension was given, make it one if not.
    if (ext[0] !== '.') {
        ext = `.${ext}`
    }

    return `
Root: HKCR; Subkey: "${ext}\\OpenWithProgids"; ValueType: none; ValueName: "${prodName}"; Flags: deletevalue uninsdeletevalue; Tasks: registerAsEditor;
Root: HKCR; Subkey: "${ext}\\OpenWithProgids"; ValueType: string; ValueName: "${prodName}${ext}"; ValueData: ""; Flags: uninsdeletevalue; Tasks: registerAsEditor;
Root: HKCR; Subkey: "${prodName}${ext}"; ValueType: string; ValueName: ""; ValueData: "${desc}"; Flags: uninsdeletekey; Tasks: registerAsEditor;
Root: HKCR; Subkey: "${prodName}${ext}\\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\\oni2.ico"; Tasks: registerAsEditor;
Root: HKCR; Subkey: "${prodName}${ext}\\shell\\open\\command"; ValueType: string; ValueName: ""; ValueData: """{app}\\${executableName}"" ""%1"""; Tasks: registerAsEditor;
`
}

_.keys(valuesToReplace).forEach(key => {
    shelljs.sed("-i", "{{" + key + "}}", valuesToReplace[key], destFile)
})

let allFilesToAddRegKeysFor = ""

packageMeta.build.fileAssociations.forEach(association => {
    association.ext.forEach(extension => {
        allFilesToAddRegKeysFor += getFileRegKey(extension, association.name)
    })
})

allFilesToAddRegKeysFor += addToEnv

shelljs.sed("-i", "{{RegistryKey}}", allFilesToAddRegKeysFor, destFile)
