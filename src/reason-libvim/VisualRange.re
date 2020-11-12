open EditorCoreTypes;

type t = {
  cursor: BytePosition.t,
  anchor: BytePosition.t,
  visualType: Types.visualType,
};

let current = () => {
  let cursor = Cursor.get();
  let range = Visual.getRange();
  let visualType = Visual.getType();

  let anchor =
    if (range.start == cursor) {
      range.stop;
    } else {
      range.start;
    };

  {cursor, anchor, visualType};
};

let cursor = ({cursor, _}) => cursor;

let range = ({cursor, anchor, _}) =>
  ByteRange.{start: cursor, stop: anchor};
