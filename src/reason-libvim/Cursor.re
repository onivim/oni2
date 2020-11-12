open EditorCoreTypes;

let toString = (cursor: BytePosition.t) =>
  Printf.sprintf(
    "Line %d column %d\n",
    cursor.line |> LineNumber.toOneBased,
    cursor.byte |> ByteIndex.toInt,
  );

let get = () => {
  BytePosition.{
    line: Native.vimCursorGetLine() |> LineNumber.ofOneBased,
    byte: Native.vimCursorGetColumn() |> ByteIndex.ofInt,
  };
};

let set = (cursor: BytePosition.t) => {
  Native.vimCursorSetPosition(
    cursor.line |> LineNumber.toOneBased,
    cursor.byte |> ByteIndex.toInt,
  );
};
