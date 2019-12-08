open Oni_Core;

type t = {
  file: string,
  location: Position.t,
  text: string,
  highlight: option((Index.t, Index.t)),
};
