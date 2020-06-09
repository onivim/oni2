open EditorCoreTypes;

type t = {
  range: Range.t,
  text: option(string),
};

let applyEdit: (~lines: array(string), t) => array(string);
