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

      // Otherwise - is it available in path?
      let merlinPath = Environment.which(merlinExecutable);
      let merlinReasonPath = Environment.which(merlinReasonExecutable);
      switch ((merlinPath, merlinReasonPath)) {
      | (Some(mp), Some(mrp)) =>
        Log.info("MerlinDiscovery::discover - found both: " ++ mp ++ " | " ++ mrp);
        complete({ocamlMerlinPath: Some(mp), ocamlMerlinReasonPath: Some(mrp)});
      | (Some(mp), None) =>
        complete({ocamlMerlinPath: Some(mp), ocamlMerlinReasonPath: None});
      | _ =>
        print_endline("Merlin not found in Path... trying esy");

        switch (Esy.getEsyPath()) {
        | None =>
          print_endline("Esy not found.");
          complete({ocamlMerlinPath: None, ocamlMerlinReasonPath: None});
        | Some(v) =>
          print_endline("Found esy: " ++ v);
          switch (Esy.getStatus(workingDirectory)) {
          
          | Ok(v) when v == Esy.Status.ReadyForDev => 
            let merlinExec = Esy.runCommand(workingDirectory, [|"where", merlinExecutable|]);
            let merlinReasonExec = Esy.runCommand(workingDirectory, [|"where", merlinReasonExecutable|]);
            switch ((merlinExec, merlinReasonExec)) {
            | (Ok(mp), Ok(mrp)) => complete({ocamlMerlinPath: Some(mp), ocamlMerlinReasonPath: Some(mrp)})
            | (Ok(mp), Error(e)) => complete({ocamlMerlinPath: Some(mp), ocamlMerlinReasonPath: None})
            | (Error(e), _) => 
              print_endline ("Error resolving esy: " ++ e);
              complete({ocamlMerlinPath: None, ocamlMerlinReasonPath: None});
            }
          | _ => 
            print_endline ("Esy project not ready");
            complete({ocamlMerlinPath: None, ocamlMerlinReasonPath: None});
          }
        };
      };
    };

  Mutex.unlock(_mutex);
  ret;
};
