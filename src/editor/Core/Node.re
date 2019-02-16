/*
 * Node.re
 *
 * Integration with NodeJs services
 */

open Rench;

let version = (~nodePath, ()) => {
    let ret = ChildProcess.spawnSync(nodePath, [|"--version"|]);
    ret.stdout;
};

let executeJs = (~nodePath, js) => {
    let ret = ChildProcess.spawnSync(nodePath, [|"-e", js|]);
    ret.stdout
}
