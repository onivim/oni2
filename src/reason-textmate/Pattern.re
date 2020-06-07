/*
 TextMateGrammar.re
 */

module Capture = {
  type t = (int, string);
};

type t =
  | Include(string, string)
  | Match(patternMatch)
  | MatchRange(matchRange)
and patternMatch = {
  matchRegex: RegExpFactory.t,
  matchName: option(string),
  captures: list(Capture.t),
}
and matchRange = {
  beginRegex: RegExpFactory.t,
  endRegex: RegExpFactory.t,
  beginCaptures: list(Capture.t),
  endCaptures: list(Capture.t),
  // The scope to append to the tokens
  name: option(string),
  // []contentName] differs from [name] in that it only
  // impacts matches _between_ the tokens.
  contentName: option(string),
  patterns: list(t),
  applyEndPatternLast: bool,
};

let show = (v: t) =>
  switch (v) {
  | Include(scope, str) => "Include(" ++ scope ++ "," ++ str ++ ")"
  | Match(_) => "Match(..)"
  | MatchRange(matchRange) =>
    let name =
      switch (matchRange.name) {
      | None => "Name: None"
      | Some(v) => "Name: " ++ v
      };
    let contentName =
      switch (matchRange.contentName) {
      | None => "ContentName: None"
      | Some(v) => "ContentName: " ++ v
      };

    "MatchRange(\n)"
    ++ " -"
    ++ name
    ++ "\n"
    ++ " -"
    ++ contentName
    ++ "\n"
    ++ " -"
    ++ RegExpFactory.show(matchRange.beginRegex)
    ++ "\n"
    ++ " -"
    ++ RegExpFactory.show(matchRange.endRegex)
    ++ "\n";
  };

module Json = {
  let string_of_yojson: (string, Yojson.Safe.t) => result(string, string) =
    (memberName, json) => {
      switch (Yojson.Safe.Util.member(memberName, json)) {
      | `String(v) => Ok(v)
      | _ => Error("Missing expected property: " ++ memberName)
      };
    };

  let bool_of_yojson: Yojson.Safe.t => bool =
    json => {
      switch (json) {
      | `Bool(v) => v
      | _ => false
      };
    };

  module Decode = {
    open Oni_Core;
    open Json.Decode;

    let capture = {
      let captureSimplified = string;
      let captureNested =
        obj(({field, _}) => {field.required("name", string)});

      one_of([
        ("simplified", captureSimplified),
        ("nested", captureNested),
      ]);
    };
  };

  let captures_of_yojson: Yojson.Safe.t => list(Capture.t) =
    json => {
      open Yojson.Safe.Util;
      let f = keyValuePair => {
        let (key, json) = keyValuePair;
        let captureGroup = int_of_string_opt(key);
        let captureName =
          json |> Oni_Core.Json.Decode.decode_value(Decode.capture);

        switch (captureGroup, captureName) {
        | (Some(n), Ok(name)) => Ok((n, name))
        | _ => Error("Invalid capture group")
        };
      };

      let fold = (prev, curr) => {
        switch (f(curr)) {
        | Ok(v) => [v, ...prev]
        | _ => prev
        };
      };

      switch (json) {
      | `Assoc(list) => List.fold_left(fold, [], list) |> List.rev
      | _ => []
      };
    };

  let regex_of_yojson = (~allowBackReferences=true, json) => {
    switch (json) {
    | `String(v) => Ok(RegExpFactory.create(~allowBackReferences, v))
    | _ => Error("Regular expression not specified")
    };
  };

  let match_of_yojson: Yojson.Safe.t => result(t, string) =
    json => {
      open Yojson.Safe.Util;
      let%bind regex = regex_of_yojson(member("match", json));

      let nameField =
        switch (member("name", json), member("contentName", json)) {
        | (`String(_), _) => "name"
        | _ => "contentName"
        };

      let name =
        switch (string_of_yojson(nameField, json)) {
        | Ok(v) => Some(v)
        | Error(_) => None
        };

      Ok(
        Match({
          matchName: name,
          matchRegex: regex,
          captures: captures_of_yojson(member("captures", json)),
        }),
      );
    };

  let rec of_yojson: (string, Yojson.Safe.t) => result(t, string) =
    (scope, json) => {
      open Yojson.Safe.Util;
      let incl = member("include", json);
      let mat = member("match", json);
      let beg = member("begin", json);

      switch (incl, mat, beg) {
      | (`String(inc), _, _) =>
        let len = String.length(inc);
        if (len > 0 && (inc.[0] == '#' || inc.[0] == '$')) {
          Ok(Include(scope, inc));
        } else {
          switch (String.index_opt(inc, '#')) {
          | None => Ok(Include(inc, "$self"))
          | Some(idx) =>
            let scope = String.sub(inc, 0, idx);
            let id = String.sub(inc, idx, len - idx);
            Ok(Include(scope, id));
          };
        };
      | (_, `String(_), _) => match_of_yojson(json)
      | (_, _, `String(_)) => matchRange_of_yojson(scope, json)
      | _ => Ok(Include("noop", "#no-op"))
      };
    }
  and matchRange_of_yojson: (string, Yojson.Safe.t) => result(t, string) =
    (scope, json) => {
      open Yojson.Safe.Util;
      let%bind beginRegex = regex_of_yojson(member("begin", json));
      let er =
        regex_of_yojson(~allowBackReferences=false, member("end", json));

      let%bind endRegex =
        switch (er) {
        | Ok(v) => Ok(v)
        | Error(_) =>
          Ok(RegExpFactory.create(~allowBackReferences=false, "\\uFFFF"))
        };

      let applyEndPatternLast =
        bool_of_yojson(member("applyEndPatternLast", json));

      let name =
        switch (string_of_yojson("name", json)) {
        | Ok(v) => Some(v)
        | _ => None
        };

      let contentName =
        switch (string_of_yojson("contentName", json)) {
        | Ok(v) => Some(v)
        | _ => None
        };

      let%bind nestedPatterns =
        switch (member("patterns", json)) {
        | `List(items) =>
          List.fold_left(
            (prev, curr) => {
              switch (prev) {
              | Error(e) => Error(e)
              | Ok(currItems) =>
                switch (of_yojson(scope, curr)) {
                | Ok(p) => Ok([p, ...currItems])
                | Error(e) => Error(e)
                }
              }
            },
            Ok([]),
            items,
          )
        | _ => Ok([])
        };

      let beginCaptureName =
        switch (member("beginCaptures", json), member("captures", json)) {
        | (`Assoc(_), _) => "beginCaptures"
        | _ => "captures"
        };

      let endCaptureName =
        switch (member("endCaptures", json), member("captures", json)) {
        | (`Assoc(_), _) => "endCaptures"
        | _ => "captures"
        };

      Ok(
        MatchRange({
          applyEndPatternLast,
          name,
          beginRegex,
          endRegex,
          beginCaptures: captures_of_yojson(member(beginCaptureName, json)),
          endCaptures: captures_of_yojson(member(endCaptureName, json)),
          contentName,
          patterns: nestedPatterns,
        }),
      );
    };

  let of_string: (string, string) => result(t, string) =
    (scope, jsonString) => {
      Yojson.Safe.from_string(jsonString) |> of_yojson(scope);
    };
};

module PlistDecoder = {
  open Plist;

  let regexp = (~allowBackReferences) =>
    fun
    | String(str) =>
      switch (RegExpFactory.create(~allowBackReferences, str)) {
      | regexp => Ok(regexp)
      | exception (Failure(error)) => Error(error ++ " in " ++ str)
      }
    | value => Error("Expected string, got: " ++ Plist.show(value));

  let capture = property("name", string);
  let captures =
    assoc(capture)
    |> map(List.map(((n, name)) => (int_of_string(n), name)));

  let include_ = scopeName =>
    dict(prop => Include(scopeName, prop.required("include", string)));
  let match =
    dict(prop =>
      Match({
        matchName: prop.optional("name", string),
        matchRegex:
          prop.required("match", regexp(~allowBackReferences=true)),
        captures: prop.withDefault("captures", captures, []),
      })
    );
  let rec matchRange = scopeName =>
    dict(prop =>
      MatchRange({
        beginRegex:
          prop.required("begin", regexp(~allowBackReferences=true)),
        endRegex: prop.required("end", regexp(~allowBackReferences=false)),
        beginCaptures: prop.withDefault("beginCaptures", captures, []),
        endCaptures: prop.withDefault("endCaptures", captures, []),
        name: prop.optional("name", string),
        contentName: prop.optional("contentName", string),
        patterns:
          prop.withDefault("patterns", array(pattern(scopeName)), []),
        applyEndPatternLast:
          prop.withDefault("applyEndPatternLast", bool, false),
      })
    )

  and pattern = scopeName =>
    oneOf([
      ("include", include_(scopeName)),
      ("match", match),
      ("matchRange", matchRange(scopeName)),
    ]);
};

let of_plist = PlistDecoder.pattern;
