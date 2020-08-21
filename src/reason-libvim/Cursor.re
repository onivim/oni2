open EditorCoreTypes;

let toString = (cursor: BytePosition.t) =>
  Printf.sprintf(
    "Line %d column %d\n",
    cursor.line |> LineNumber.toOneBased,
    cursor.byte |> ByteIndex.toInt,
  );

//let getLine = () => Index.fromOneBased(Native.vimCursorGetLine());
//let getColumn = () => Index.fromZeroBased(Native.vimCursorGetColumn());

//let get = () => create(~line=getLine(), ~column=getColumn());

let get = () => {
  BytePosition.{
    line: Native.vimCursorGetLine() |> LineNumber.ofOneBased,
    byte: Native.vimCursorGetColumn() |> ByteIndex.ofInt,
  };
};
//  Location.create(
//    ~line=Index.fromOneBased(Native.vimCursorGetLine()),
//    ~column=Index.fromZeroBased(Native.vimCursorGetColumn()),
//  );

let set = (cursor: BytePosition.t) => {
  //  let previousTopLine = Native.vimWindowGetTopLine();
  //  let previousLeft = Native.vimWindowGetLeftColumn();
  Native.vimCursorSetPosition(
    cursor.line |> LineNumber.toOneBased,
    cursor.byte |> ByteIndex.toInt,
  );
};
