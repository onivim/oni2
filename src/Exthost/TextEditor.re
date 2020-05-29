open Oni_Core;

module CursorStyle = {
  type t =
    | Hidden // 0
    | Blink // 1
    | Smooth // 2
    | Phase // 3
    | Expand // 4
    | Solid; // 5;

  let toInt =
    fun
    | Hidden => 0
    | Blink => 1
    | Smooth => 2
    | Phase => 3
    | Expand => 4
    | Solid => 5;

  let encode = cursorStyle => cursorStyle |> toInt |> Json.Encode.int;
};

module LineNumbersStyle = {
  type t =
    | Off
    | On
    | Relative;

  let toInt =
    fun
    | Off => 0
    | On => 1
    | Relative => 2;

  let encode = lineNumbersStyle =>
    lineNumbersStyle |> toInt |> Json.Encode.int;
};

module ResolvedConfiguration = {
  type t = {
    tabSize: int,
    indentSize: int,
    insertSpaces: int,
    cursorStyle: CursorStyle.t,
    lineNumbers: LineNumbersStyle.t,
  };

  let encode = resolvedConfig =>
    Json.Encode.(
      obj([
        ("tabSize", resolvedConfig.tabSize |> int),
        ("indentSize", resolvedConfig.indentSize |> int),
        ("insertSpaces", resolvedConfig.insertSpaces |> int),
        ("cursorStyle", resolvedConfig.cursorStyle |> CursorStyle.encode),
        (
          "lineNumbers",
          resolvedConfig.lineNumbers |> LineNumbersStyle.encode,
        ),
      ])
    );
};

module AddData = {
  type t = {
    id: string,
    documentUri: Uri.t,
    options: ResolvedConfiguration.t,
    // TODO:
    // selections: list(Selection.t),
    // visibleRanges: list(Range.t),
    // editorPosition: option(EditorViewColumn.t),
  };

  let encode = ({id, documentUri, options}) =>
    Json.Encode.(
      obj([
        ("id", id |> string),
        ("documentUri", documentUri |> Uri.encode),
        ("options", options |> ResolvedConfiguration.encode),
        // TODO:
        ("selections", [] |> list(int)),
        ("visibleRanges", [] |> list(int)),
      ])
    );
};
