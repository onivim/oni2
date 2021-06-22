open Oni_Core;

module CursorStyle = {
  [@deriving show]
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
  [@deriving show]
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
  [@deriving show]
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
  [@deriving show]
  type t = {
    id: string,
    documentUri: Uri.t,
    options: ResolvedConfiguration.t,
    selections: list(Selection.t),
    // TODO:
    // visibleRanges: list(Range.t),
    // editorPosition: option(EditorViewColumn.t),
  };

  let encode = ({id, documentUri, options, selections}) =>
    Json.Encode.(
      obj([
        ("id", id |> string),
        ("documentUri", documentUri |> Uri.encode),
        ("options", options |> ResolvedConfiguration.encode),
        ("selections", selections |> list(Selection.encode)),
        // TODO:
        ("visibleRanges", [] |> list(int)),
      ])
    );
};

module SelectionChangeEvent = {
  type t = {
    selections: list(Selection.t),
    source: option(string),
  };

  let encode = ({selections, source}) =>
    Json.Encode.(
      obj([
        ("selections", selections |> list(Selection.encode)),
        ("source", source |> nullable(string)),
      ])
    );
};

module PropertiesChangeData = {
  type t = {
    selections: option(SelectionChangeEvent.t),
    // options: IResolvedTextEditorConfiguration
    // visibleRanges: IRange[]
  };

  let encode = ({selections}) =>
    Json.Encode.(
      obj([
        ("selections", selections |> nullable(SelectionChangeEvent.encode)),
      ])
    );
};
