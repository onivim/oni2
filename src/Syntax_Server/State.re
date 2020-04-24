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

let anyPendingWork = state => getActiveHighlighters(state) != [];

let bufferEnter = (id: int, state: t) => {
  // TODO: Don't add duplicates...
  let visibleBuffers = [id, ...state.visibleBuffers];

  {...state, visibleBuffers};
};

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
             highlights
             |> NativeSyntaxHighlights.getUpdatedLines
             |> List.map(line => {
                  let tokenColors =
                    NativeSyntaxHighlights.getTokensForLine(highlights, line);
                  let bufferId = curr;
                  Protocol.TokenUpdate.create(~bufferId, ~line, tokenColors);
                })
           })
        |> Option.value(~default=[]);

      [tokenUpdatesForBuffer, ...acc];
    },
    [],
    state.visibleBuffers,
  )
  |> List.flatten;
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
             Some({lines: update.lines, version: update.version});
           } else {
             None;
           }
         | Some({lines, _}) =>
           if (update.isFull) {
             Some({lines: update.lines, version: update.version});
           } else {
             let newLines =
               ArrayEx.replace(
                 ~replacement=update.lines,
                 ~start=update.startLine |> Index.toZeroBased,
                 ~stop=update.endLine |> Index.toZeroBased,
                 lines,
               );
             Some({lines: newLines, version: update.version});
           }
         }
       );
  {...state, bufferInfo};
};

let getBuffer = (~bufferId, state) => {
  IntMap.find_opt(bufferId, state.bufferInfo);
};

let bufferUpdate = (~scope, ~bufferUpdate: BufferUpdate.t, state) => {
  let state = applyBufferUpdate(~update=bufferUpdate, state);

  state
  |> getBuffer(~bufferId=bufferUpdate.id)
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
                   ~bufferUpdate,
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

let updateVisibility = (visibility: list((int, list(Range.t))), state) => {
  let highlightsMap =
    visibility
    |> List.fold_left(
         (acc, curr) => {
           let (bufferId, ranges) = curr;

           let updateVisibility =
             fun
             | None => None
             | Some(hl) =>
               Some(NativeSyntaxHighlights.updateVisibleRanges(ranges, hl));

           IntMap.update(bufferId, updateVisibility, acc);
         },
         state.highlightsMap,
       );

  {...state, highlightsMap};
};
