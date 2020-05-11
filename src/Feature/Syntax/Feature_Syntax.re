open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Syntax;

module BufferMap = IntMap;
module LineMap = IntMap;

let highlight = (~scope, ~theme, ~grammars, lines) => {
  let grammarRepository =
    Textmate.GrammarRepository.create(scope =>
      Oni_Syntax.GrammarRepository.getGrammar(~scope, grammars)
    );

  let tokenizerJob =
    Oni_Syntax.TextmateTokenizerJob.create(
      ~scope,
      ~theme,
      ~grammarRepository,
      lines,
    )
    |> Job.tick(~budget=Some(0.1)); // 100ms

  let len = Array.length(lines);
  let result = Array.make(len, []);

  for (i in 0 to len - 1) {
    let tokens =
      Oni_Syntax.TextmateTokenizerJob.getTokenColors(i, tokenizerJob);
    result[i] = tokens;
  };

  result;
};

[@deriving show({with_path: false})]
type msg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
  | ServerFailedToStart(string)
  | ServerStopped
  | TokensHighlighted([@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t));
//  | Service(Service_Syntax.serverMsg);

type outmsg =
  | Nothing
  | ServerError(string);

type t = {
  maybeSyntaxClient: option(Oni_Syntax_Client.t),
  highlights: BufferMap.t(LineMap.t(list(ColorizedToken.t))),
  ignoredBuffers: BufferMap.t(bool),
};

let empty = {
  ignoredBuffers: BufferMap.empty,
  highlights: BufferMap.empty,
  maybeSyntaxClient: None,
};

let noTokens = [];

module ClientLog = (val Oni_Core.Log.withNamespace("Oni2.Feature.Syntax"));

let getTokens = (~bufferId: int, ~line: Index.t, {highlights, _}) => {
  highlights
  |> BufferMap.find_opt(bufferId)
  |> OptionEx.flatMap(LineMap.find_opt(line |> Index.toZeroBased))
  |> Option.value(~default=noTokens);
};

let getSyntaxScope =
    (~bufferId: int, ~line: Index.t, ~bytePosition: int, bufferHighlights) => {
  let tokens = getTokens(~bufferId, ~line, bufferHighlights);

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
    (
      ~bufferId: int,
      ~line: int,
      ~tokens: list(ColorizedToken.t),
      {highlights, ignoredBuffers} as prev: t,
    ) => {
  let updateLineMap = (lineMap: LineMap.t(list(ColorizedToken.t))) => {
    LineMap.update(line, _ => Some(tokens), lineMap);
  };

  let highlights =
    BufferMap.update(
      bufferId,
      fun
      | None => Some(updateLineMap(LineMap.empty))
      | Some(v) => Some(updateLineMap(v)),
      highlights,
    );
  {...prev, ignoredBuffers, highlights};
};

let setTokens = (tokenUpdates: list(Protocol.TokenUpdate.t), highlights: t) => {
  Protocol.TokenUpdate.(
    tokenUpdates
    |> List.fold_left(
         (acc, curr) => {
           setTokensForLine(
             ~bufferId=curr.bufferId,
             ~line=curr.line,
             ~tokens=curr.tokenColors,
             acc,
           )
         },
         highlights,
       )
  );
};

let ignore = (~bufferId, bufferHighlights) => {
  let ignoredBuffers =
    bufferHighlights.ignoredBuffers |> BufferMap.add(bufferId, true);

  {...bufferHighlights, ignoredBuffers};
};

// When there is a buffer update, shift the lines to match
let handleUpdate = (bufferUpdate: BufferUpdate.t, bufferHighlights) =>
  if (BufferMap.mem(bufferUpdate.id, bufferHighlights.ignoredBuffers)) {
    bufferHighlights;
  } else {
    let highlights =
      BufferMap.update(
        bufferUpdate.id,
        fun
        | None => None
        | Some(lineMap) => {
            let startPos = bufferUpdate.startLine |> Index.toZeroBased;
            let endPos = bufferUpdate.endLine |> Index.toZeroBased;
            Some(
              LineMap.shift(
                ~default=v => v,
                ~startPos,
                ~endPos,
                ~delta=Array.length(bufferUpdate.lines) - (endPos - startPos),
                lineMap,
              ),
            );
          },
        bufferHighlights.highlights,
      );
    {...bufferHighlights, highlights};
  };

let update: (t, msg) => (t, outmsg) =
  (highlights: t, msg) =>
    switch (msg) {
    | TokensHighlighted(tokens) => (setTokens(tokens, highlights), Nothing)
    | ServerFailedToStart(msg) => (highlights, ServerError(msg))
    | ServerStarted(client) => (
        {...highlights, maybeSyntaxClient: Some(client)},
        Nothing,
      )
    | ServerStopped => ({...highlights, maybeSyntaxClient: None}, Nothing)
    //| Service(_) => (highlights, Nothing)
    };

let subscription =
    (
      ~configuration,
      ~languageInfo,
      ~setup,
      ~tokenTheme,
      ~bufferVisibility,
      {maybeSyntaxClient, _},
    ) => {
  let getBufferSubscriptions = client => {
    bufferVisibility
    |> List.map(((buffer, visibleRanges)) => {
         Service_Syntax.Sub.buffer(~client, ~buffer, ~visibleRanges)
         |> Isolinear.Sub.map(
              fun
              | Service_Syntax.ReceivedHighlights(updates) =>
                TokensHighlighted(updates),
            )
       });
  };

  let bufferSubscriptions =
    maybeSyntaxClient
    |> Option.map(getBufferSubscriptions)
    |> Option.value(~default=[]);

  let serverSubscription =
    Service_Syntax.Sub.server(
      ~configuration,
      ~languageInfo,
      ~setup,
      ~tokenTheme,
    )
    |> Isolinear.Sub.map(
         fun
         | Service_Syntax.ServerStarted(client) => ServerStarted(client)
         | Service_Syntax.ServerFailedToStart(msg) => ServerFailedToStart(msg)
         | Service_Syntax.ServerClosed => ServerStopped,
       );

  Isolinear.Sub.batch([serverSubscription, ...bufferSubscriptions]);
};

module Effect = {
  let bufferUpdate = (~bufferUpdate, {maybeSyntaxClient, _}) => {
    Isolinear.Effect.create(~name="feature.syntax.bufferUpdate", () => {
      maybeSyntaxClient
      |> Option.iter(syntaxClient => {
           Oni_Syntax_Client.notifyBufferUpdate(~bufferUpdate, syntaxClient)
         })
    });
  };
};
