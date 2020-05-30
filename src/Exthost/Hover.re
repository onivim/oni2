open Oni_Core;

// Based on Hover type, defined here:
// https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/editor/common/modes.ts#L237
type t = {
  contents: list(MarkdownString.t),
  range: option(OneBasedRange.t),
};

let decode =
  Json.Decode.(
    obj(({field, _}) =>
      {
        contents: field.required("contents", list(MarkdownString.decode)),
        range: field.optional("range", OneBasedRange.decode),
      }
    )
  );
