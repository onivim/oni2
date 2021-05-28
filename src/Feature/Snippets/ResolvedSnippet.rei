open EditorCoreTypes;

type snippet;
type t = snippet;

module Placeholder: {
  type t;

  let hasAny: snippet => bool;

  let text: (~index: int, t) => option(string);

  type positions =
    | Positions(list(BytePosition.t))
    | Ranges(list(ByteRange.t));

  let positions:
    (~placeholders: t, ~index: int, snippet) => option(positions);

  let next: (~placeholder: int, t) => int;

  let previous: (~placeholder: int, t) => int;

  let initial: t => int;

  let final: t => int;
};

let placeholders: t => Placeholder.t;

let resolve:
  (
    ~getVariable: string => option(string),
    ~prefix: string,
    ~postfix: string,
    ~indentationSettings: Oni_Core.IndentationSettings.t,
    Snippet.t
  ) =>
  t;

let updatePlaceholder: (~index: int, ~text: string, t) => t;

let getFirstLineIndexWithPlaceholder: (~index: int, snippet) => option(int);

let getPlaceholderCountForLine: (~index: int, ~line: int, snippet) => int;

let lineCount: t => int;

let toLines: t => array(string);
