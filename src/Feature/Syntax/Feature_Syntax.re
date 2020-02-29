open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Syntax;

module BufferMap = IntMap;
module LineMap = IntMap;

[@deriving show({with_path: false})]
type msg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
  | ServerStopped
  | TokensHighlighted([@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t))
  | BufferUpdated([@opaque] BufferUpdate.t)
  | Service(Service_Syntax.msg);

type t = BufferMap.t(LineMap.t(list(ColorizedToken.t)));

let empty = BufferMap.empty;

let noTokens = [];

module ClientLog = (val Oni_Core.Log.withNamespace("Oni2.Feature.Syntax"));

let getTokens = (~bufferId: int, ~line: Index.t, highlights: t) => {
  highlights
  |> BufferMap.find_opt(bufferId)
  |> OptionEx.flatMap(LineMap.find_opt(line |> Index.toZeroBased))
  |> Option.value(~default=noTokens);
};

let getSyntaxScope =
    (~bufferId: int, ~line: Index.t, ~bytePosition: int, highlights: t) => {
  let tokens = getTokens(~bufferId, ~line, highlights);

  let rec loop = (syntaxScope, currentTokens) => {
    ColorizedToken.(
      switch (currentTokens) {
      // Reached the end... return what we have
      | [] => syntaxScope
      | [{index, syntaxScope, _}, ...rest] when index <= bytePosition =>
        loop(syntaxScope, rest)
      // Gone past the relevant tokens, return
      // the scope we ended up with
      | _ => syntaxScope
      }
    );
  };

  loop(SyntaxScope.none, tokens);
};

let setTokensForLine =
    (bufferId: int, line: int, tokens: list(ColorizedToken.t), highlights: t) => {
  let updateLineMap = (lineMap: LineMap.t(list(ColorizedToken.t))) => {
    LineMap.update(line, _ => Some(tokens), lineMap);
  };

  BufferMap.update(
    bufferId,
    fun
    | None => Some(updateLineMap(LineMap.empty))
    | Some(v) => Some(updateLineMap(v)),
    highlights,
  );
};

let setTokens = (tokenUpdates: list(Protocol.TokenUpdate.t), highlights: t) => {
  Protocol.TokenUpdate.(
    tokenUpdates
    |> List.fold_left(
         (acc, curr) => {
           setTokensForLine(curr.bufferId, curr.line, curr.tokenColors, acc)
         },
         highlights,
       )
  );
};

// When there is a buffer update, shift the lines to match
let handleUpdate = (bufferUpdate: BufferUpdate.t, highlights: t) => {
  BufferMap.update(
    bufferUpdate.id,
    fun
    | None => None
    | Some(lineMap) =>
      Some(
        LineMap.shift(
          ~default=v => v,
          ~startPos=bufferUpdate.startLine |> Index.toZeroBased,
          ~endPos=bufferUpdate.endLine |> Index.toZeroBased,
          ~delta=Array.length(bufferUpdate.lines),
          lineMap,
        ),
      ),
    highlights,
  );
};

let update = (highlights: t, msg) =>
  switch (msg) {
  | TokensHighlighted(tokens) => setTokens(tokens, highlights)
  | BufferUpdated(update) when !update.isFull =>
    handleUpdate(update, highlights)
  | ServerStarted(_client) => highlights
  | ServerStopped => highlights
  | BufferUpdated(_update) => highlights
  | Service(_) => highlights
  };

let subscription = (~enabled, ~quitting, ~languageInfo, ~setup, _highlights) =>
  if (enabled && !quitting) {
    Service_Syntax.Sub.create(~languageInfo, ~setup)
    |> Isolinear.Sub.map(
         fun
         | Service_Syntax.ServerStarted(client) => ServerStarted(client)
         | Service_Syntax.ServerClosed => ServerStopped
         | Service_Syntax.ReceivedHighlights(hl) => TokensHighlighted(hl),
       );
  } else {
    Isolinear.Sub.none;
  };
