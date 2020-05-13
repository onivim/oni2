/*
 State.re

 State modelled for the syntax server
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Syntax;

module Ext = Oni_Extensions;

type logFunc = string => unit;

type bufferInfo = {
  lines: array(string),
  version: int,
  scope: string,
};

type t = {
  useTreeSitter: bool,
  setup: option(Setup.t),
  bufferInfo: IntMap.t(bufferInfo),
  languageInfo: Ext.LanguageInfo.t,
  treesitterRepository: TreesitterRepository.t,
  grammarRepository: GrammarRepository.t,
  theme: TokenTheme.t,
  visibleBuffers: list(int),
  highlightsMap: IntMap.t(NativeSyntaxHighlights.t),
};

let empty = {
  useTreeSitter: false,
  setup: None,
  bufferInfo: IntMap.empty,
  visibleBuffers: [],
  highlightsMap: IntMap.empty,
  theme: TokenTheme.empty,
  languageInfo: Ext.LanguageInfo.initial,
  grammarRepository: GrammarRepository.empty,
  treesitterRepository: TreesitterRepository.empty,
};

let initialize = (~log, languageInfo, setup, state) => {
  ...state,
  languageInfo,
  grammarRepository: GrammarRepository.create(~log, languageInfo),
  treesitterRepository: TreesitterRepository.create(~log, languageInfo),
  setup: Some(setup),
};

module Constants = {
  let defaultScope = "source.text";
};

let getVisibleBuffers = state => state.visibleBuffers;

let getVisibleHighlighters = state => {
  state.visibleBuffers
  |> List.map(b => IntMap.find_opt(b, state.highlightsMap))
  |> List.filter_map(v => v);
};

let getActiveHighlighters = state => {
  getVisibleHighlighters(state)
  |> List.filter(hl => NativeSyntaxHighlights.anyPendingWork(hl));
};

module Internal = {
  let getBuffer = (~bufferId, state) => {
    IntMap.find_opt(bufferId, state.bufferInfo);
  };

  let getBufferScope = (~bufferId: int, state: t) => {
    state.bufferInfo
    |> IntMap.find_opt(bufferId)
    |> Option.map(({scope, _}) => scope)
    |> Option.value(~default=Constants.defaultScope);
  };
};

let anyPendingWork = state => getActiveHighlighters(state) != [];

let updateTheme = (theme, state) => {
  let highlightsMap =
    IntMap.map(
      NativeSyntaxHighlights.updateTheme(theme),
      state.highlightsMap,
    );

  {...state, theme, highlightsMap};
};

let setUseTreeSitter = (useTreeSitter, state) => {
  {...state, useTreeSitter};
};

let doPendingWork = state => {
  let highlightsMap =
    List.fold_left(
      (prev, curr) =>
        IntMap.update(
          curr,
          fun
          | None => None
          | Some(highlighter) =>
            Some(NativeSyntaxHighlights.doWork(highlighter)),
          prev,
        ),
      state.highlightsMap,
      state.visibleBuffers,
    );

  {...state, highlightsMap};
};

let getTokenUpdates = state => {
  List.fold_left(
    (acc, curr) => {
      let tokenUpdatesForBuffer =
        state.highlightsMap
        |> IntMap.find_opt(curr)
        |> Option.map(highlights => {
             let tokenUpdates =
               highlights
               |> NativeSyntaxHighlights.getUpdatedLines
               |> List.map(line => {
                    let tokenColors =
                      NativeSyntaxHighlights.getTokensForLine(
                        highlights,
                        line,
                      );
                    Protocol.TokenUpdate.create(~line, tokenColors);
                  });

             (curr, tokenUpdates);
           })
        |> Option.value(~default=(curr, []));

      [tokenUpdatesForBuffer, ...acc];
    },
    [],
    state.visibleBuffers,
  );
};

let clearTokenUpdates = state => {
  let highlightsMap =
    List.fold_left(
      (acc, curr) => {
        acc
        |> IntMap.update(
             curr,
             fun
             | None => None
             | Some(highlight) =>
               Some(NativeSyntaxHighlights.clearUpdatedLines(highlight)),
           )
      },
      state.highlightsMap,
      state.visibleBuffers,
    );

  {...state, highlightsMap};
};

let applyBufferUpdate = (~update: BufferUpdate.t, state) => {
  let bufferInfo =
    state.bufferInfo
    |> IntMap.update(update.id, current =>
         switch (current) {
         | None =>
           if (update.isFull) {
             Some({
               scope: Constants.defaultScope,
               lines: update.lines,
               version: update.version,
             });
           } else {
             None;
           }
         | Some({scope, lines, _}) =>
           if (update.isFull) {
             Some({scope, lines: update.lines, version: update.version});
           } else {
             let newLines =
               ArrayEx.replace(
                 ~replacement=update.lines,
                 ~start=update.startLine |> Index.toZeroBased,
                 ~stop=update.endLine |> Index.toZeroBased,
                 lines,
               );
             Some({scope, lines: newLines, version: update.version});
           }
         }
       );
  {...state, bufferInfo};
};

let bufferUpdate = (~bufferUpdate: BufferUpdate.t, state) => {
  let state = applyBufferUpdate(~update=bufferUpdate, state);
  let scope = Internal.getBufferScope(~bufferId=bufferUpdate.id, state);

  state
  |> Internal.getBuffer(~bufferId=bufferUpdate.id)
  |> Option.map(({lines, _}) => {
       let highlightsMap =
         IntMap.update(
           bufferUpdate.id,
           current =>
             switch (current) {
             | None =>
               let getTextmateGrammar = scope =>
                 GrammarRepository.getGrammar(
                   ~scope,
                   state.grammarRepository,
                 );

               let getTreesitterScope = scope =>
                 TreesitterRepository.getScopeConverter(
                   ~scope,
                   state.treesitterRepository,
                 );

               Some(
                 NativeSyntaxHighlights.create(
                   ~useTreeSitter=state.useTreeSitter,
                   ~theme=state.theme,
                   ~scope,
                   ~getTreesitterScope,
                   ~getTextmateGrammar,
                   lines,
                 ),
               );
             | Some(v) =>
               Some(NativeSyntaxHighlights.update(~bufferUpdate, ~lines, v))
             },
           state.highlightsMap,
         );
       {...state, highlightsMap};
     })
  |> Option.to_result(~none="Unable to apply update");
};

let updateBufferVisibility =
    (~bufferId, ~ranges: list(Range.t), {highlightsMap, _} as state) => {
  let updateVisibility =
    fun
    | None => None
    | Some(hl) =>
      Some(NativeSyntaxHighlights.updateVisibleRanges(ranges, hl));

  let highlightsMap =
    highlightsMap |> IntMap.update(bufferId, updateVisibility);

  {...state, highlightsMap};
};

let bufferEnter =
    (~bufferId: int, ~filetype: string, ~lines, ~visibleRanges, state: t) => {
  let exists = List.exists(id => id == bufferId, state.visibleBuffers);
  let visibleBuffers =
    if (exists) {
      state.visibleBuffers;
    } else {
      [bufferId, ...state.visibleBuffers];
    };

  let scope =
    Oni_Extensions.LanguageInfo.getScopeFromLanguage(
      state.languageInfo,
      filetype,
    )
    |> Option.value(~default=Constants.defaultScope);

  let bufferInfo =
    state.bufferInfo
    |> IntMap.update(
         bufferId,
         fun
         | None => Some({lines, version: (-1), scope})
         | Some(bufInfo) => Some({...bufInfo, scope}),
       );

  let state = {...state, bufferInfo, visibleBuffers};

  let update =
    BufferUpdate.{
      id: bufferId,
      isFull: true,
      lines,
      startLine: Index.zero,
      endLine: Index.(zero + Array.length(lines)),
      version: 0,
    };

  state
  |> applyBufferUpdate(~update)
  |> updateBufferVisibility(~bufferId, ~ranges=visibleRanges);
};

let bufferLeave =
    (
      ~bufferId: int,
      {bufferInfo, visibleBuffers, highlightsMap, _} as state: t,
    ) => {
  let bufferInfo = IntMap.remove(bufferId, bufferInfo);
  let visibleBuffers = List.filter(id => id != bufferId, visibleBuffers);
  let highlightsMap = IntMap.remove(bufferId, highlightsMap);

  {...state, bufferInfo, visibleBuffers, highlightsMap};
};
