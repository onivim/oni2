/*
 * MerlinDiscovery.re
 *
 * This module assists in locating the current merlin executable to use
 */

open Oni_Core;

open Rench;

type t = {
  ocamlMerlinPath: option(string),
  ocamlMerlinReasonPath: option(string),
};

let show = (v: t) => {
    let omp = switch (v.ocamlMerlinPath) {
    | Some(v) => v
    | None => "None"
    };

    let omrp = switch (v.ocamlMerlinReasonPath) {
    | Some(v) => v
    | None => "None"
    };

    Printf.sprintf("ocamlMerlinPath: %s\n ocamlMerlinReasonPath: %s\n", omp, omrp);
};

let _cache: Hashtbl.t(string, t) = Hashtbl.create(8);
let _mutex = Mutex.create();

let merlinExecutable =
  switch (Revery.Environment.os) {
  | Windows => "ocamlmerlin.exe"
  | _ => "ocamlmerlin"
  };

let merlinReasonExecutable =
  switch (Revery.Environment.os) {
  | Windows => "ocamlmerlin-reason.exe"
  | _ => "ocamlmerlin-reason"
  };

let whichOrWhere =
  switch (Revery.Environment.os) {
  | Windows => "where"
  | _ => "which"
  }

let default = {ocamlMerlinPath: None, ocamlMerlinReasonPath: None};

let discover = (workingDirectory: string) => {
  Mutex.lock(_mutex);

  let ret =
    switch (Hashtbl.find_opt(_cache, workingDirectory)) {
    | Some(v) => v
    | None =>
      let complete = (v: t) => {
        Hashtbl.add(_cache, workingDirectory, v);
        v;
      };
        
        // Try esy first
        let (ocamlMerlinPath, ocamlMerlinReasonPath) = switch (Esy.getEsyPath()) {
        | None =>
          Log.info("Esy not found.");
          (None, None)
        | Some(v) =>
          Log.info("Found esy: " ++ v);
          switch (Esy.getStatus(workingDirectory)) {
          | Ok(v) when v == Esy.Status.ReadyForDev =>
            let merlinExec =
              Esy.runCommand(
                workingDirectory,
                [|whichOrWhere, merlinExecutable|],
              );
            let merlinReasonExec =
              Esy.runCommand(
                workingDirectory,
                [|whichOrWhere, merlinReasonExecutable|],
              );
            switch (merlinExec, merlinReasonExec) {
            | (Ok(mp), Ok(mrp)) =>
              (Some(mp), Some(mrp))
            | (Ok(mp), Error(e)) =>
              (Some(mp), None)
            | (Error(e), _) =>
              Log.info("Error resolving esy: " ++ e);
              (None, None)
            };
          | _ =>
            Log.info("Esy project not ready");
            (None, None)
          };
        };

      // If we weren't able to find with 'esy', lets fallback to looking in the environment

      let (ocamlMerlinPath, ocamlMerlinReasonPath) = switch ((ocamlMerlinPath, ocamlMerlinReasonPath)) {
      | (Some(mp), Some(rmp)) => (Some(mp), Some(rmp))
      | (Some(mp), None) => (Some(mp), Environment.which(merlinReasonExecutable))
      | (None, Some(rmp)) => (Environment.which(merlinExecutable), Some(rmp))
      | (None, None) => (Environment.which(merlinExecutable), Environment.which(merlinReasonExecutable))
      };

      
      let result = {
        ocamlMerlinPath,
        ocamlMerlinReasonPath,
      };
      Log.info("Merlin discovery result: " ++ show(result));
      complete(result);
    };

  Mutex.unlock(_mutex);
  ret;
};
