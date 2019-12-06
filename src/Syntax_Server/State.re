/*
 State.re

 State modelled for the syntax server
 */

open Oni_Core;
open Oni_Core.Utility;
open Oni_Syntax;

module Ext = Oni_Extensions;

module List = Utility.List;

type t = {
  setup: option(Setup.t),
  languageInfo: Ext.LanguageInfo.t,
  grammarRepository: GrammarRepository.t,
  theme: TokenTheme.t,
  visibleBuffers: list(int),
  highlightsMap: IntMap.t(NativeSyntaxHighlights.t),
};

let empty = {
  setup: None,
  visibleBuffers: [],
  highlightsMap: IntMap.empty,
  theme: TokenTheme.empty,
  languageInfo: Ext.LanguageInfo.empty,
  grammarRepository: GrammarRepository.empty,
};

let initialize = (~log, languageInfo, setup, state) => {
  ...state,
  languageInfo,
  grammarRepository: GrammarRepository.create(~log, languageInfo),
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

let updateVisibleBuffers = (buffers, state) => {
  let visibleBuffers =
    List.map(
      v => {
        let (id, _) = v;
        id;
      },
      buffers,
    );

  let highlightsMap =
    List.fold_left(
      (prev, curr) => {
        let (bufferId, ranges) = curr;
        IntMap.update(
          bufferId,
          oldV =>
            switch (oldV) {
            | None => None
            | Some(v) =>
              Some(NativeSyntaxHighlights.updateVisibleRanges(ranges, v))
            },
          prev,
        );
      },
      state.highlightsMap,
      buffers,
    );

  {...state, visibleBuffers, highlightsMap};
};

/*let getTokensForLine = (v: t, bufferId: int, line: int) => {
    switch (IntMap.find_opt(bufferId, v.highlightsMap)) {
    | Some(v) => NativeSyntaxHighlights.getTokensForLine(v, line)
    | None => []
    }};
  }*/

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
  let highlightsMap = List.fold_left(
  (acc, curr) => {
    acc
    |> IntMap.update(curr, fun
    | None => None
    | Some(highlight) => Some(NativeSyntaxHighlights.clearUpdatedLines(highlight))
    );
  }, state.highlightsMap, state.visibleBuffers);

  {
  ...state,
  highlightsMap,
  }
}

let bufferUpdate =
    //      ~configuration,
    //      ~scope,
    //      ~getTreeSitterScopeMapper,
    //      ~getTextmateGrammar,
    (~bufferUpdate: BufferUpdate.t, ~lines: array(string), state) => {
  let highlightsMap =
    IntMap.update(
      bufferUpdate.id,
      current =>
        switch (current) {
        | None =>
          let getTextmateGrammar = (scope) => GrammarRepository.getGrammar(~scope, state.grammarRepository);
          Some(
            NativeSyntaxHighlights.create(
              //              ~configuration,
              ~bufferUpdate,
              ~theme=state.theme,
              ~scope="source.reason",
              //              ~getTreeSitterScopeMapper,
              ~getTextmateGrammar,
              lines,

            ),
          )
        | Some(v) =>
          Some(NativeSyntaxHighlights.update(~bufferUpdate, ~lines, v))
        },
      state.highlightsMap,
    );
  {...state, highlightsMap};
};
