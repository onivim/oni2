/*
 * Node.re
 *
 * Integration with NodeJs services
 */

open Rench;

let getNodePath = () => "D:/oni2/vendor/node-v10.15.1/win-x64/node.exe";

let version = () => {
    let ret = ChildProcess.spawnSync(getNodePath(), [|"--version"|]);
    ret.stdout;
};
