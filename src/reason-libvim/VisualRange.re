open EditorCoreTypes;

type t = {
  range: Range.t,
  visualType: Types.visualType,
};

let create = (~range, ~visualType, ()) => {range, visualType};
