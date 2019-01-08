/*
 * NeovimProcess.re
 *
 * Management of running `nvim` processes
 */

open Rench;

type t = {
    pid: int,
};

let version = (
    ~neovimPath: string,
) => {
    let ret = ChildProcess.spawnSync(neovimPath, [|"--version"|]);
    ret.stdout;
};


let start = (
    ~neovimPath: string,
    ~args: array(string),
) => {

    /* let nvimBinaryPath = Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH"); */

    ChildProcess.spawn(neovimPath, args);
};

