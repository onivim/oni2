/*
 * NeovimProcess.re
 *
 * Management of running `nvim` processes
 */

open Rench;

type t = {
    pid: int,
};


let start = (
    ~neovimPath: string,
) => {

    let proc = ChildProcess.spawn(neovimPath, [|"--embed"|], {env: Unix.environment()});

    let ret: t = {

    };

};

