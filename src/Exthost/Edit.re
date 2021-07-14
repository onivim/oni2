open Oni_Core;

module SingleEditOperation = {
  [@deriving show]
  type t = {
    range: OneBasedRange.t,
    text: option(string),
    forceMoveMarkers: bool,
  };

  let deltaLineCount = ({range, text, _}) => {
    let originalLineCount = range.endLineNumber - range.startLineNumber + 1;
    let newLineCount =
      text
      |> Option.map(lines =>
           lines |> String.split_on_char('\n') |> List.length
         )
      |> Option.value(~default=0);
    newLineCount - originalLineCount;
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          range: field.required("range", OneBasedRange.decode),
          text: field.withDefault("text", None, nullable(string)),
          forceMoveMarkers:
            field.withDefault("forceMoveMarkers", false, bool),
        }
      )
    );
};
