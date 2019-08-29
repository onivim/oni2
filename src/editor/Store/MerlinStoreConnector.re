/*
 * VimStoreConnector.re
 *
 * This module connects vim to the Store:
 * - Translates incoming vim notifications into Actions
 * - Translates Actions into Effects that should run against vim
 */

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

module Log = Core.Log;
module Zed_utf8 = Core.ZedBundled;

open Rench;

module MerlinProtocol = {
[@deriving yojson({strict: false})]
type oneBasedLine = int;

[@deriving yojson({strict: false})]
type oneBasedCharacter = int;

module Position = {
  [@deriving yojson({strict: false})]
  type t = {
    line: oneBasedLine,
    col: oneBasedCharacter,
  };
};

  [@deriving yojson({strict: false})]
  type typeEnclosingResultItem = {
    tail: string,
    [@key "type"]
    enclosingType: string,
    [@key "start"]
    startPosition: Position.t,
    [@key "end"]
    endPosition: Position.t,
  };

  [@deriving yojson({strict: false})]
  type typeEnclosingResult = list(typeEnclosingResultItem);

  [@deriving yojson({strict: false})]
  type errorResultItem = {
    message: string,
    [@key "type"]
    errorType: string,
    [@key "start"]
    startPosition: Position.t,
    [@key "end"]
    endPosition: Position.t,
  };

  [@deriving yojson({strict: false})]
  type errorResult = list(errorResultItem);

  [@deriving yojson({strict: false})]
  type completionResultItem = {
    name: string,
    kind: string,
    desc: string,
  };

  [@deriving yojson({strict: false})]
  type completionResult = {entries: list(completionResultItem)};

  type t = result(Yojson.Safe.json, string);

let getValueAsString = (json: Yojson.Safe.json) => {
  json |> Yojson.Safe.Util.member("value") |> Yojson.Safe.Util.to_string;
};
  let parse = (json: Yojson.Safe.json) => {
    let responseClass =
      json |> Yojson.Safe.Util.member("class") |> Yojson.Safe.Util.to_string;

    let isFailure =
      String.equal(responseClass, "error")
      || String.equal(responseClass, "failure")
      || String.equal(responseClass, "exception");
    let ret: t=
      isFailure ?
        Error(getValueAsString(json)) :
        Ok(json |> Yojson.Safe.Util.member("value"));
    ret;
  };
};

module MerlinProtocolConverter = {
  
  let toModelDiagnostics = (errors: MerlinProtocol.errorResult) => {
    let f = (err: MerlinProtocol.errorResultItem) => {
      Model.Diagnostics.Diagnostic.create(
        ~range=Core.Range.ofPositions(
          ~startPosition=Core.Types.Position.ofInt1(err.startPosition.line, err.startPosition.col + 1),
          ~endPosition=Core.Types.Position.ofInt1(err.endPosition.line, err.endPosition.col + 1),
          ()
        ),
        ()
      );
      };
      
    List.map(f, errors);
  };
};

module Merlin = {
  type t = {
    ocamlMerlinPath: option(string),
    ocamlMerlinReasonPath: option(string),
  };
  
  let default = {
    ocamlMerlinPath: Some("C:\\Users\\bryphe\\.esy\\3_\\i\\opam__s__merlin-opam__c__3.2.2-862fb62c\\bin\\ocamlmerlin.exe"),
    ocamlMerlinReasonPath: Some("C:\\Users\\bryphe\\.esy\\3_\\i\\esy_ocaml__s__reason-3.4.0-ac7b3839\\bin\\ocamlmerlin-reason.exe"),
  };

  let pendingRequest = ref(false);

  let getErrors = (filePath: string, input: array(string), cb) => {

    if (!pendingRequest^) {

    switch (default.ocamlMerlinPath) {
    | Some (v) =>

      Thread.create(() => {
      pendingRequest := true;
      let (pstdin, stdin) = Unix.pipe();
      let (stdout, pstdout) = Unix.pipe();
      let (stderr, pstderr) = Unix.pipe();

      Unix.set_close_on_exec(pstdin);
      Unix.set_close_on_exec(stdin);
      Unix.set_close_on_exec(pstdout);
      Unix.set_close_on_exec(stdout);
      Unix.set_close_on_exec(pstderr);
      Unix.set_close_on_exec(stderr);
      
      let pid = Unix.create_process(
        v,
        [|v, "single", "errors", "-filename", "src/editor/Store/StoreThread.re"|],
        pstdin, pstdout, pstdout
      );

      Unix.close(pstdout);
      Unix.close(pstdin);
      Unix.close(pstderr);
      
      let stdIn = Unix.out_channel_of_descr(stdin);
      let stdout = Unix.in_channel_of_descr(stdout);

      let i = ref(0);
      let len = Array.length(input);

      while (i^ < len) {
        output_string(stdIn, input[i^] ++ "\n");
        Thread.yield();
        incr(i);
      }
      
      close_out_noerr(stdIn);
      
      let json = Yojson.Safe.from_channel(stdout);
      let result = MerlinProtocol.parse(json);

      switch (result) {
      | Ok(v) => 
          print_endline("RESULT: " ++ Yojson.Safe.to_string(v));
          let errors = MerlinProtocol.errorResult_of_yojson(v);

          switch (errors) {
          | Ok(v) => cb(v)
          | Error(e) => print_endline("ERROR: " ++ e);
          }
      | Error(e) => print_endline("ERROR: " ++ e);
      };

      
      
      
      pendingRequest := false;

      }, ());

      Ok("winning");
   | None => Error("No merlin instance available");
  };
  } else {
    Error("other request pending");
  }
};
};


let start =
    () => {
  let (stream, dispatch) = Isolinear.Stream.create();
  let modelChangedEffect =
      (buffers: Model.Buffers.t, bu: Core.Types.BufferUpdate.t) =>
    Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
      switch (Model.Buffers.getBuffer(bu.id, buffers)) {
      | None => ()
      | Some(v) =>
         let cb = (err) => {
         let modelDiagnostics = MerlinProtocolConverter.toModelDiagnostics(err);
         print_endline ("Got " ++ string_of_int(List.length(modelDiagnostics)) ++ " errors");
         Revery.App.runOnMainThread(() => dispatch(Model.Actions.DiagnosticsSet(v, "merlin", modelDiagnostics)));
         }
        let lines = Model.Buffer.getLines(v);
        let _ = Merlin.getErrors("test.re", lines, cb);
      }
    );

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.BufferUpdate(bu) => (
        state,
        modelChangedEffect(state.buffers, bu),
      )
    | _ => (state, Isolinear.Effect.none);
    };
  };

  (updater, stream);
};
