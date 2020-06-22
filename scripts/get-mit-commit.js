/* get-mit-commit.js


 *


 * We are dual-licensing MIT commits 18 months after they hit master


 * This script gets the commit ID for the latest MIT commit


 */

const cp = require("child_process")

const date = new Date()

const eighteenMonthsAgo = new Date(date.setMonth(date.getMonth() - 18))

console.log(eighteenMonthsAgo.toString())

const month = eighteenMonthsAgo.toLocaleString("en-us", { month: "short" })

const day = eighteenMonthsAgo.getDate()

const year = eighteenMonthsAgo.getFullYear()

const ret = cp
    .execSync(`git rev-list -1 --before="${month} ${day} ${year}" master`)
    .toString("utf-8")

console.log(ret.trim())
