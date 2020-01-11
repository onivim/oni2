/*
 State.re

 State modelled for the syntax server
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Syntax;

module Ext = Oni_Extensions;

module List = Utility.List;

type logFunc = string => unit;

type t = {
  configuration: Configuration.t,
  setup: option(Setup.t),
  languageInfo: Ext.LanguageInfo.t,
  treesitterRepository: TreesitterRepository.t,
  grammarRepository: GrammarRepository.t,
  theme: TokenTheme.t,
  visibleBuffers: list(int),
  bufferIdToScope: IntMap.t(string),
  keywordMap: KeywordMap.t,
  highlightsMap: IntMap.t(NativeSyntaxHighlights.t),
};

let empty = {
  configuration: Configuration.default,
  setup: None,
  visibleBuffers: [],
  highlightsMap: IntMap.empty,
  theme: TokenTheme.empty,
  languageInfo: Ext.LanguageInfo.initial,
  grammarRepository: GrammarRepository.empty,
  treesitterRepository: TreesitterRepository.empty,
  bufferIdToScope: IntMap.empty,
  keywordMap: KeywordMap.empty,
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

let updateConfiguration = (configuration, state) => {
  {...state, configuration};
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

  // Apply any newly discovered keywords
  let keywordMap =
    List.fold_left(
      (acc, curr) => {
        let discoveredKeywords =
          state.highlightsMap
          |> IntMap.find_opt(curr)
          |> Option.map(NativeSyntaxHighlights.getDiscoveredKeywords);

        let scope = state.bufferIdToScope |> IntMap.find_opt(curr);

        Option.map2(
          (keywords: list(string), scope: string) => {
            KeywordMap.set(
              ~bufferId=curr,
              ~scope,
              // TODO: Get lines from keywords
              ~line=0,
              ~words=keywords,
              acc,
            );
          },
          discoveredKeywords,
          scope,
        )
        |> Option.value(~default=acc);
      },
      state.keywordMap,
      state.visibleBuffers,
    );

  // TODO: Clear keyword discovery
  {...state, keywordMap, highlightsMap};
};

let getKeywordsForScope = (~scope, state) => {
  state.keywordMap
  |> KeywordMap.get(
    ~scope
  );
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

let bufferUpdate =
    (~scope, ~bufferUpdate: BufferUpdate.t, ~lines: array(string), state) => {
  let bufferIdToScope =
    IntMap.update(
      bufferUpdate.id,
      fun
      | None => Some(scope)
      | Some(_) => Some(scope),
      state.bufferIdToScope,
    );

  let highlightsMap =
    IntMap.update(
      bufferUpdate.id,
      current =>
        switch (current) {
        | None =>
          let getTextmateGrammar = scope =>
            GrammarRepository.getGrammar(~scope, state.grammarRepository);

          let getTreesitterScope = scope =>
            TreesitterRepository.getScopeConverter(
              ~scope,
              state.treesitterRepository,
            );

          Some(
            NativeSyntaxHighlights.create(
              ~configuration=state.configuration,
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
  {...state, bufferIdToScope, highlightsMap};
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
