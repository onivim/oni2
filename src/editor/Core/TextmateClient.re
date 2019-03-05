/*
 * TextmateClient.re
 *
 * This is a client-side API for integrating with our textmate 'server'.
 *
 * Long-term, it'd be ideal to instead implement this API in an in-proc fashion -
 * for example, if vscode-textmate was ported to native ReasonML
 */

module CoreUtility = Utility;
open Reason_jsonrpc;

type scopeInfo = {
  scopeName: string,
  path: string,
};

type initializationInfo = list(scopeInfo);

type tokenizeResult = {
  startIndex: int,
  endIndex: int,
  scopes: list(string),
};

module ColorizedToken = {
  /* From:
   * https://github.com/Microsoft/vscode-textmate/blob/master/src/main.ts
   */
  let languageId_mask = 0b00000000000000000000000011111111;
  let token_type_mask = 0b00000000000000000000011100000000;
  let font_style_mask = 0b00000000000000000011100000000000;
  let foreground_mask = 0b00000000011111111100000000000000;
  let background_mask = 0b11111111100000000000000000000000;

  let languageid_offset = 0;
  let token_type_offset = 8;
  let font_style_offset = 11;
  let foreground_offset = 14;
  let background_offset = 23;

  type t = {
    index: int,
    foregroundColor: int,
    backgroundColor: int,
  };

  let getForegroundColor: int => int =
    v => {
      (v land foreground_mask) lsr foreground_offset;
    };

  let getBackgroundColor: int => int =
    v => {
      (v land background_mask) lsr background_offset;
    };

  let create: (int, int) => t =
    (idx, v) => {
      index: idx,
      foregroundColor: getForegroundColor(v) - 1,
      backgroundColor: getBackgroundColor(v) - 1,
    };
};

let parseTokenizeResultItem = (json: Yojson.Safe.json) => {
  switch (json) {
  | `List([`Int(startIndex), `Int(endIndex), `List(jsonScopes)]) =>
    let scopes = List.map(s => Yojson.Safe.to_string(s), jsonScopes);
    {startIndex, endIndex, scopes};
  | _ => {startIndex: (-1), endIndex: (-1), scopes: []}
  };
};

let rec parseColorResult = (json: list(Yojson.Safe.json)) => {
  switch (json) {
  | [`Int(v1), `Int(v2), ...tail] => [
      ColorizedToken.create(v1, v2),
      ...parseColorResult(tail),
    ]
  | _ => []
  };
};

module TokenizationResult = {
  exception TokenParseException(string);

  type tokenizationLineResult = {
    line: int,
    tokens: list(ColorizedToken.t),
  };

  type t = {
    bufferId: int,
    version: int,
    lines: list(tokenizationLineResult),
  };

  let parseTokenizationLineResult = (json: Yojson.Safe.json) => {
    switch (json) {
    | `Assoc([("line", `Int(line)), ("tokens", `List(tokensJson))]) => {
        line,
        tokens: parseColorResult(tokensJson),
      }
    | _ => raise(TokenParseException("Unexpected tokenization line result"))
    };
  };

  let of_yojson = (json: Yojson.Safe.json) => {
    switch (json) {
    | `Assoc([
        ("bufferId", `Int(bufferId)),
        ("version", `Int(version)),
        ("lines", `List(linesJson)),
      ]) => {
        bufferId,
        version,
        lines: List.map(parseTokenizationLineResult, linesJson),
      }
    | _ => raise(TokenParseException("Unexpected token result"))
    };
  };
};

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

type onScopeLoaded = string => unit;
let defaultScopeLoaded: onScopeLoaded = _ => ();

type onColorMap = ColorMap.t => unit;
let defaultColorMap: onColorMap = _ => ();

type onTokens = TokenizationResult.t => unit;
let defaultOnTokens: onTokens = _ => ();

type t = {
  process: NodeProcess.t,
  rpc: Rpc.t,
  onColorMap,
};

let emptyJsonValue = `Assoc([]);

let start =
    (
      ~onClosed=defaultCallback,
      ~onColorMap=defaultColorMap,
      ~onInitialized=defaultCallback,
      ~onScopeLoaded=defaultScopeLoaded,
      ~onTokens=defaultOnTokens,
      setup: Setup.t,
      initializationInfo,
    ) => {
  let process = NodeProcess.start(setup, setup.textmateServicePath);

  let onNotification = (n: Notification.t, _) => {
    switch (n.method, n.params) {
    | ("initialized", _) => onInitialized()
    | ("textmate/scopeLoaded", `String(s)) => onScopeLoaded(s)
    | ("textmate/publishTokens", json) => {
      onTokens(TokenizationResult.of_yojson(json))
    }
    | _ => ()
    };
  };

  let onRequest = (_, _) => Ok(emptyJsonValue);

  let rpc =
    Rpc.start(
      ~onNotification,
      ~onRequest,
      ~onClose=onClosed,
      process.stdout,
      process.stdin,
    );

  let mapScopeInfoToJson = (v: scopeInfo) => {
    (v.scopeName, `String(v.path));
  };

  let initInfo = initializationInfo |> List.map(mapScopeInfoToJson);
  Rpc.sendNotification(rpc, "initialize", `Assoc(initInfo));

  {process, rpc, onColorMap};
};

let getpid = (v: t) => v.process.pid;

let preloadScope = (v: t, scopeName: string) => {
  Rpc.sendNotification(v.rpc, "textmate/preloadScope", `String(scopeName));
};

let pump = (v: t) => Rpc.pump(v.rpc);

let setTheme = (v: t, themePath: string) => {
  Rpc.sendRequest(
    v.rpc,
    "textmate/setTheme",
    `Assoc([("path", `String(themePath))]),
    (response, _) =>
    switch (response) {
    | Ok(json) => v.onColorMap(ColorMap.ofJson(json))
    | _ => prerr_endline("Unable to load theme")
    }
  );
};

type tokenizeLineResult = {
  tokens: list(tokenizeResult),
  colors: list(ColorizedToken.t),
};

let notifyBufferUpdate = (v: t, bufUpdate: Types.BufferUpdate.t) => {
  Rpc.sendNotification(
    v.rpc,
    "textmate/bufferUpdate",
    /* TODO: Don't hardcode this */
    `List([
      `String("source.reason"),
      Types.BufferUpdate.to_yojson(bufUpdate),
    ]),
  );
};

let tokenizeLineSync = (v: t, scopeName: string, line: string) => {
  let gotResponse = ref(false);
  let result: ref(option(tokenizeLineResult)) = ref(None);

  Rpc.sendRequest(
    v.rpc,
    "textmate/tokenizeLine",
    `Assoc([("scopeName", `String(scopeName)), ("line", `String(line))]),
    (response, _) => {
      let tokens: option(tokenizeLineResult) =
        switch (response) {
        | Ok(
            `Assoc([("tokens", `List(items)), ("colors", `List(colors))]),
          ) =>
          let tokens = List.map(parseTokenizeResultItem, items);
          let colors = parseColorResult(colors);

          Some({tokens, colors});
        | _ => None
        };

      gotResponse := true;
      result := tokens;
    },
  );

  CoreUtility.waitForCondition(() => {
    Rpc.pump(v.rpc);
    gotResponse^;
  });

  result^;
};

let close = (v: t) => {
  Rpc.sendNotification(v.rpc, "exit", emptyJsonValue);

  let result = Unix.waitpid([], v.process.pid);
  result;
};
