open EditorCoreTypes;

type t = {
  range: ByteRange.t,
  visualType: Types.visualType,
};

let create = (~range, ~visualType, ()) => {range, visualType};
