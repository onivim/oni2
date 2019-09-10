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

  let esyExecutable = switch(Revery.Environment.os) {
  | Windows => "esy.cmd"
  | _ => "esy";
  };

  let getEsyPath = () => {
    switch (_esyPath^) {
    | Some(v) => Some(v)
    | None => {

      let esyPath = Environment.which(esyExecutable);
      _esyPath := esyPath;
      esyPath;
    }
    }
  };
