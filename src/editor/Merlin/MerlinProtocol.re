/*
 * MerlinProtocol.re
 *
 * This module contains types for the merlin protocol:
 * https://github.com/ocaml/merlin/blob/master/doc/dev/PROTOCOL.md
 */

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

  type t = result(Yojson.Safe.t, string);

  let getValueAsString = (json: Yojson.Safe.t) => {
    json |> Yojson.Safe.Util.member("value") |> Yojson.Safe.Util.to_string;
  };
  let parse = (json: Yojson.Safe.t) => {
    let responseClass =
      json |> Yojson.Safe.Util.member("class") |> Yojson.Safe.Util.to_string;

    let isFailure =
      String.equal(responseClass, "error")
      || String.equal(responseClass, "failure")
      || String.equal(responseClass, "exception");
    let ret: t =
      isFailure
        ? Error(getValueAsString(json))
        : Ok(json |> Yojson.Safe.Util.member("value"));
    ret;
  };
