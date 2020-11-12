const fs = require("fs")
const path = require("path")

const commitId = process.argv[2]
const destPath = process.argv[3]
console.log(`Commit: ${commitId}`)
console.log(`Destination Folder: ${destPath}`)

const templateMd = path.join(__dirname, "template.md")
const destinationMd = path.join(destPath, "README.md")

const inputLines = fs.readFileSync(templateMd, "utf8")

const lines = inputLines.replace("%%COMMITID%%", commitId)

console.log(lines)

console.log("Updating README.md...")
fs.writeFileSync(destinationMd, lines, "utf8")

console.log("Updating package.json license field...")
let packageJsonPath = path.join(destPath, "package.json")

let packageJson = JSON.parse(fs.readFileSync(packageJsonPath, "utf8"))
packageJson["license"] = "MIT"
fs.writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, "  "))
