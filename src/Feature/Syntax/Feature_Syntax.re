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

module Tokens = {
  let getAt = (~byteIndex as orig, tokens) => {
    let byteIndex = ByteIndex.toInt(orig);
    let rec loop = (lastToken, tokens: list(ThemeToken.t)) => {
      switch (tokens) {
      | [] => lastToken
      | [token] => token.index <= byteIndex ? Some(token) : lastToken
      | [token, ...tail] =>
        if (token.index > byteIndex) {
          lastToken;
        } else {
          loop(Some(token), tail);
        }
      };
    };

    loop(None, tokens);
  };
};

module Configuration = {
  open Oni_Core;
  open Config.Schema;

  let eagerMaxLines = setting("syntax.eagerMaxLines", int, ~default=500);

  let eagerMaxLineLength =
    setting("syntax.eagerMaxLineLength", int, ~default=1000);

  module Experimental = {
    let treeSitter = setting("experimental.treeSitter", bool, ~default=false);
  };
};

module Contributions = {
  let configuration = [
    Configuration.eagerMaxLines.spec,
    Configuration.eagerMaxLineLength.spec,
    Configuration.Experimental.treeSitter.spec,
  ];
};

module Internal = {
  let eagerHighlight =
      (~grammars, ~scope, ~config: Config.resolver, ~theme, lines) => {
    let maxLines = Configuration.eagerMaxLines.get(config);
    let maxLineLength = Configuration.eagerMaxLineLength.get(config);

    let len = min(Array.length(lines), maxLines);

    let numberOfLinesToHighlight = {
      let rec iter = idx =>
        if (idx >= len) {
          idx;
        } else if (String.length(lines[idx]) > maxLineLength) {
          idx;
        } else {
          iter(idx + 1);
        };

      iter(0);
    };

    if (numberOfLinesToHighlight == 0) {
      [||];
    } else {
      let linesToHighlight = Array.sub(lines, 0, numberOfLinesToHighlight);
      let highlights = highlight(~scope, ~theme, ~grammars, linesToHighlight);
      highlights;
    };
  };
};

[@deriving show({with_path: false})]
type msg =
  | ServerStarted
  | ServerInitialized([@opaque] Oni_Syntax_Client.t)
  | ServerFailedToStart(string)
  | ServerStopped
  | TokensHighlighted({
      bufferId: int,
      tokens: [@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t),
    });

type outmsg =
  | Nothing
  | ServerError(string);

type t = {
  maybeSyntaxClient: option(Oni_Syntax_Client.t),
  highlights: BufferMap.t(LineMap.t(list(ThemeToken.t))),
  ignoredBuffers: BufferMap.t(bool),
};

let empty = {
  ignoredBuffers: BufferMap.empty,
  highlights: BufferMap.empty,
  maybeSyntaxClient: None,
};

let noTokens = [];

module ClientLog = (val Oni_Core.Log.withNamespace("Oni2.Feature.Syntax"));

let getTokens =
    (~bufferId: int, ~line: EditorCoreTypes.LineNumber.t, {highlights, _}) => {
  highlights
  |> BufferMap.find_opt(bufferId)
  |> OptionEx.flatMap(
       LineMap.find_opt(line |> EditorCoreTypes.LineNumber.toZeroBased),
     )
  |> Option.value(~default=noTokens);
};

let getAt =
    (~bufferId, ~bytePosition: EditorCoreTypes.BytePosition.t, highlights) => {
  let tokens = getTokens(~bufferId, ~line=bytePosition.line, highlights);
  Tokens.getAt(~byteIndex=bytePosition.byte, tokens);
};

let getSyntaxScope =
    (~bytePosition: BytePosition.t, ~bufferId: int, bufferHighlights) => {
  let tokens =
    getTokens(~bufferId, ~line=bytePosition.line, bufferHighlights);
  let byte = ByteIndex.toInt(bytePosition.byte);

  let rec loop = (syntaxScope, currentTokens) => {
    ThemeToken.(
      switch (currentTokens) {
      // Reached the end... return what we have
      | [] => syntaxScope
      | [{index, syntaxScope, _}, ...rest] when index <= byte =>
        loop(syntaxScope, rest)
      // Gone past the relevant tokens, return
      // the scope we ended up with
      | _ => syntaxScope
      }
    );
  };

  loop(SyntaxScope.none, tokens);
};

let isSyntaxServerRunning = ({maybeSyntaxClient, _}) => {
  maybeSyntaxClient != None;
};

let setTokensForLine =
    (
      ~bufferId: int,
      ~line: int,
      ~tokens: list(ThemeToken.t),
      {highlights, ignoredBuffers, _} as prev: t,
    ) => {
  let updateLineMap = (lineMap: LineMap.t(list(ThemeToken.t))) => {
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

let setTokens =
    (bufferId, tokenUpdates: list(Protocol.TokenUpdate.t), highlights: t) => {
  Protocol.TokenUpdate.(
    tokenUpdates
    |> List.fold_left(
         (acc, curr) => {
           setTokensForLine(
             ~bufferId,
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
let handleUpdate =
    (
      ~grammars,
      ~scope,
      ~theme,
      ~config: Config.resolver,
      ~bufferUpdate: BufferUpdate.t,
      ~markerUpdate: MarkerUpdate.t,
      bufferHighlights,
    ) =>
  if (BufferMap.mem(bufferUpdate.id, bufferHighlights.ignoredBuffers)) {
    bufferHighlights;
  } else if (bufferUpdate.version == 1 && bufferUpdate.isFull) {
    // Eager syntax highlighting - on the very first buffer update,
    // we'll synchronously calculate syntax highlights for an initial
    // chunk of the buffer.
    let newHighlights = ref(bufferHighlights);
    let highlights =
      Internal.eagerHighlight(
        ~scope,
        ~config,
        ~theme,
        ~grammars,
        bufferUpdate.lines,
      );

    let len = Array.length(highlights);

    for (i in 0 to len - 1) {
      newHighlights :=
        setTokensForLine(
          ~bufferId=bufferUpdate.id,
          ~line=i,
          ~tokens=highlights[i],
          newHighlights^,
        );
    };
    newHighlights^;
  } else {
    // Otherwise, if not a full update, we'll pre-emptively shift highlights
    // to give immediate feedback.
    let highlights =
      BufferMap.update(
        bufferUpdate.id,
        fun
        | None => None
        | Some(lineMap) => {
            let shiftLines = (~afterLine, ~delta, lineMap) => {
              let lineNumber =
                EditorCoreTypes.LineNumber.toZeroBased(afterLine);
              LineMap.shift(
                ~default=v => v,
                ~startPos=lineNumber,
                ~endPos=lineNumber,
                ~delta,
                lineMap,
              );
            };

            let shiftCharacters =
                (
                  ~line: EditorCoreTypes.LineNumber.t,
                  ~afterByte: ByteIndex.t,
                  ~deltaBytes: int,
                  ~afterCharacter as _,
                  ~deltaCharacters as _,
                  lineMap,
                ) => {
              lineMap
              |> LineMap.update(
                   line |> EditorCoreTypes.LineNumber.toZeroBased,
                   Option.map(tokens => {
                     let afterIndex = afterByte |> ByteIndex.toInt;
                     tokens
                     |> List.map((token: ThemeToken.t) =>
                          if (token.index >= afterIndex) {
                            {...token, index: token.index + deltaBytes};
                          } else {
                            token;
                          }
                        );
                   }),
                 );
            };

            let clearLine = (~line as _, model) => model;

            Some(
              MarkerUpdate.apply(
                ~clearLine,
                ~shiftLines,
                ~shiftCharacters,
                markerUpdate,
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
    | TokensHighlighted({bufferId, tokens}) => (
        setTokens(bufferId, tokens, highlights),
        Nothing,
      )
    | ServerStarted => (highlights, Nothing)
    | ServerFailedToStart(msg) => (highlights, ServerError(msg))
    | ServerInitialized(client) => (
        {...highlights, maybeSyntaxClient: Some(client)},
        Nothing,
      )
    | ServerStopped => ({...highlights, maybeSyntaxClient: None}, Nothing)
    };

let subscription =
    (
      ~buffers: Feature_Buffers.model,
      ~config: Config.resolver,
      ~grammarInfo,
      ~languageInfo,
      ~setup,
      ~tokenTheme,
      ~bufferVisibility,
      {maybeSyntaxClient, ignoredBuffers, _},
    ) => {
  let getBufferSubscriptions = client => {
    bufferVisibility
    |> List.filter(((buffer, _)) =>
         !BufferMap.mem(Oni_Core.Buffer.getId(buffer), ignoredBuffers)
         && !Feature_Buffers.isLargeFile(buffers, buffer)
       )
    |> List.map(((buffer, visibleRanges)) => {
         Service_Syntax.Sub.buffer(
           ~client,
           ~buffer,
           ~languageInfo,
           ~visibleRanges,
         )
         |> Isolinear.Sub.map(
              fun
              | Service_Syntax.ReceivedHighlights(tokens) => {
                  let bufferId = Buffer.getId(buffer);
                  TokensHighlighted({bufferId, tokens});
                },
            )
       });
  };

  let bufferSubscriptions =
    maybeSyntaxClient
    |> Option.map(getBufferSubscriptions)
    |> Option.value(~default=[]);

  let serverSubscription =
    Service_Syntax.Sub.server(
      ~useTreeSitter=Configuration.Experimental.treeSitter.get(config),
      ~grammarInfo,
      ~setup,
      ~tokenTheme,
    )
    |> Isolinear.Sub.map(
         fun
         | Service_Syntax.ServerStarted => ServerStarted
         | Service_Syntax.ServerInitialized(client) =>
           ServerInitialized(client)
         | Service_Syntax.ServerFailedToStart(msg) => ServerFailedToStart(msg)
         | Service_Syntax.ServerClosed => ServerStopped,
       );

  Isolinear.Sub.batch([serverSubscription, ...bufferSubscriptions]);
};

module Effect = {
  let bufferUpdate = (~bufferUpdate, {maybeSyntaxClient, _}) => {
    maybeSyntaxClient
    |> Option.map(client => {
         Service_Syntax.Effect.bufferUpdate(~client, ~bufferUpdate)
       })
    |> Option.value(~default=Isolinear.Effect.none);
  };
};
