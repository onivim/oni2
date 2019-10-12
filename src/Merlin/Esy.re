/*
 * Esy.re
 *
 * This module enables some cursory integration with Esy, like:
 * - Finding the `esy` binary
 * - Checking project status
 * -
 */
open Rench;

let _esyPath = ref(None);

let esyExecutable =
  switch (Revery.Environment.os) {
  | Windows => "esy.cmd"
  | _ => "esy"
  };

let getEsyPath = () => {
  switch (_esyPath^) {
  | Some(v) => Some(v)
  | None =>
    let esyPath = Environment.which(esyExecutable);
    _esyPath := esyPath;
    esyPath;
  };
};

module Status = {

[@deriving show]
type t =
| ReadyForDev
| NotReady
};

let runCommand: (string, array(string)) => result(string, string) = (path: string, command: array(string)) => {
  let esyPath = getEsyPath();
  switch (esyPath) {
  | None => Error("No esy available");
  | Some(esy) => {
      let ret = ChildProcess.spawnSync(~cwd=Some(path), esy, command);
      if (ret.exitCode == 0) {
        Ok(String.trim(ret.stdout))
      } else {
        Error(String.trim(ret.stderr));
      }
  }
  };
};

let getStatus: string => result(Status.t, string) = (path: string) => {
  let result = runCommand(path, [|"status"|]);
  switch (result) {
  | Error(_) as err => err
  | Ok(v) => 
        let json = Yojson.Safe.from_string(v);
        switch (Yojson.Safe.Util.member("isProjectReadyForDev", json)) {
        | `Bool(v) when v == true => Ok(ReadyForDev)
        | _ => Ok(NotReady);
        }
    
  }
};

