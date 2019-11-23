open Oni_Core.Types;

type t = {
  file: string,
  location: Position.t,
  text: string,
  highlight: option((Index.t, Index.t))
};