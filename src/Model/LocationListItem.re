open EditorCoreTypes;

type t = {
  file: string,
  location: Location.t,
  text: string,
  highlight: option((Index.t, Index.t)),
};
