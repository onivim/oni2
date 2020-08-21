open EditorCoreTypes;

type t = {
  range: CharacterRange.t,
  visualType: Types.visualType,
};

let create = (~range, ~visualType, ()) => {range, visualType};
