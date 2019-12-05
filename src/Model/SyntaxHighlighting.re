/*
 * SyntaxHighlighting.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */
open Oni_Core;
open Oni_Syntax;

module List = Utility.List;

type t = {
  visibleBuffers: list(int),
  highlightsMap: IntMap.t(NativeSyntaxHighlights.t),
};

let empty = {visibleBuffers: [], highlightsMap: IntMap.empty};

let getVisibleHighlighters = (v: t) => {
  v.visibleBuffers
  |> List.map(b => IntMap.find_opt(b, v.highlightsMap))
  |> List.filter_map(v => v);
};

let getActiveHighlighters = (v: t) => {
  getVisibleHighlighters(v)
  |> List.filter(hl => NativeSyntaxHighlights.anyPendingWork(hl));
};

let anyPendingWork = (v: t) => {
  switch (getActiveHighlighters(v)) {
  | [] => false
  | _ => true
  };
};

let updateTheme = (theme, v: t) => {
  let highlightsMap =
    IntMap.map(
      oldV => {NativeSyntaxHighlights.updateTheme(theme, oldV)},
      v.highlightsMap,
    );

  {...v, highlightsMap};
};

let doPendingWork = (v: t) => {
  let highlightsMap =
    List.fold_left(
      (prev, curr) =>
        IntMap.update(
          curr,
          oldV =>
            switch (oldV) {
            | None => None
            | Some(v) => Some(NativeSyntaxHighlights.doWork(v))
            },
          prev,
        ),
      v.highlightsMap,
      v.visibleBuffers,
    );

  {...v, highlightsMap};
};

let updateVisibleBuffers = (buffers, v: t) => {
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
      v.highlightsMap,
      buffers,
    );

  {visibleBuffers, highlightsMap};
};

let getTokensForLine = (v: t, bufferId: int, line: int) => {
  switch (IntMap.find_opt(bufferId, v.highlightsMap)) {
  | Some(v) => NativeSyntaxHighlights.getTokensForLine(v, line)
  | None => []
  };
};

let onBufferUpdate =
    (
      ~configuration,
      ~scope,
      ~getTreeSitterScopeMapper,
      ~getTextmateGrammar,
      ~bufferUpdate: BufferUpdate.t,
      ~lines: array(string),
      ~theme: TokenTheme.t,
      v: t,
    ) => {
  let highlightsMap =
    IntMap.update(
      bufferUpdate.id,
      current =>
        switch (current) {
        | None =>
          Some(
            NativeSyntaxHighlights.create(
              ~configuration,
              ~theme,
              ~scope,
              ~getTreeSitterScopeMapper,
              ~getTextmateGrammar,
              lines,
            ),
          )
        | Some(v) =>
          Some(NativeSyntaxHighlights.update(~bufferUpdate, ~lines, v))
        },
      v.highlightsMap,
    );
  let ret: t = {...v, highlightsMap};

  ret;
};
