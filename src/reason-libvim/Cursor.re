open EditorCoreTypes;

type t =
  Location.t = {
    line: Index.t,
    column: Index.t,
  };

let create = Location.create;

let show = cursor =>
  Printf.sprintf(
    "Line %d column %d\n",
    Index.toOneBased(cursor.line),
    Index.toZeroBased(cursor.column),
  );

let getLine = () => Index.fromOneBased(Native.vimCursorGetLine());

let getColumn = () => Index.fromZeroBased(Native.vimCursorGetColumn());
let get = () => create(~line=getLine(), ~column=getColumn());

let getLocation = () =>
  Location.create(
    ~line=Index.fromOneBased(Native.vimCursorGetLine()),
    ~column=Index.fromZeroBased(Native.vimCursorGetColumn()),
  );

let setLocation = (~line, ~column) => {
  let previousTopLine = Native.vimWindowGetTopLine();
  let previousLeft = Native.vimWindowGetLeftColumn();

  let lastLocation = getLocation();
  Native.vimCursorSetPosition(
    Index.toOneBased(line),
    Index.toZeroBased(column),
  );
  let newLocation = getLocation();
  let newTopLine = Native.vimWindowGetTopLine();
  let newLeft = Native.vimWindowGetLeftColumn();

  if (!Location.(lastLocation == newLocation)) {
    Event.dispatch(newLocation, Listeners.cursorMoved);
  };

  if (previousTopLine !== newTopLine) {
    Event.dispatch(newTopLine, Listeners.topLineChanged);
  };

  if (previousLeft !== newLeft) {
    Event.dispatch(newLeft, Listeners.leftColumnChanged);
  };
};

let onMoved = f => Event.add(f, Listeners.cursorMoved);

let set = ({line, column}) => setLocation(~line, ~column);
