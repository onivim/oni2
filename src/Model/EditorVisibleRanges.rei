open EditorCoreTypes;
open Oni_Core;

type t = list((int, list(Range.t)));

let getVisibleBuffersAndRanges: State.t => t;
