/*
 * SyntaxHighlighting2.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */
open Oni_Core;
open Oni_Core.Types;
open Oni_Syntax;

type t = {
   visibleBuffers: list(int),
   highlightsMap: IntMap.t(NativeSyntaxHighlights.t),
};

let empty = {
 visibleBuffers: [],
 highlightsMap: IntMap.empty,
}

let updateVisibleBuffers = (buffers: list(int), v: t) => {
  ...v,
  visibleBuffers: buffers,
};

let getTokensForLine = (v: t, bufferId: int, line: int) => {
  switch (IntMap.find_opt(bufferId, v.highlightsMap)) {
  | Some(v) => NativeSyntaxHighlights.getTokensForLine(v, line)
  | None => []
  };
};

let onBufferUpdate = (
   ~getTreeSitterScopeMapper,
   ~bufferUpdate: BufferUpdate.t, 
   ~lines: array(string),
   ~theme: TextMateTheme.t,
   v: t
   ) => {
            let highlightsMap = IntMap.update(
              bufferUpdate.id,
              (current) => switch(current) {
              | None => Some(NativeSyntaxHighlights.create(
                ~theme,
                ~getTreeSitterScopeMapper,
                lines,
              ))
              | Some(v) => Some(NativeSyntaxHighlights.update(
                ~bufferUpdate,
                ~lines,
                v))
              },
              v.highlightsMap
            );
           let ret: t = {
            ...v,
            highlightsMap: highlightsMap,
           };

           ret;
   };
