/*
 * MerlinDiscovery.re
 *
 * This module assists in locating the current merlin executable to use
 */

open Rench;

type t = {ocamlMerlinPath: option(string)};

let _cache: Hashtbl.t(string, t) = Hashtbl.create(8);
let _mutex = Mutex.create();

let merlinExecutable =
  switch (Revery.Environment.os) {
  | Windows => "ocamlmerlin.exe"
  | _ => "ocamlmerlin"
  };

let default = {ocamlMerlinPath: None};

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
      switch (merlinPath) {
      | Some(v) =>
        print_endline("FOUND PATH: " ++ v);
        complete({ocamlMerlinPath: Some(v)});
      | None =>
        print_endline("Merlin not found in Path... trying esy");

        switch (Esy.getEsyPath()) {
        | None =>
          print_endline("Esy not found.");
          complete({ocamlMerlinPath: None});
        | Some(v) =>
          print_endline("Found esy: " ++ v);
          switch (Esy.getStatus(workingDirectory)) {
          
          | Ok(v) when v == Esy.Status.ReadyForDev => 
            let merlinExecutable = Esy.runCommand(workingDirectory, [|"where", merlinExecutable|]);
            switch (merlinExecutable) {
            | Ok(v) => complete({ocamlMerlinPath: Some(v)})
            | Error(e) => 
              print_endline ("Error resolving esy: " ++ e);
              complete({ocamlMerlinPath: None});
            }
          | _ => 
            print_endline ("Esy project not ready");
            complete({ocamlMerlinPath: None});
          }
        };
      };
    };

  Mutex.unlock(_mutex);
  ret;
};
