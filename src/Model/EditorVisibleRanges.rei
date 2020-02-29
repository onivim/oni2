open EditorCoreTypes;

type t = list((int, list(Range.t)));

let getVisibleBuffersAndRanges: State.t => t;
