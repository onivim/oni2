exception Found(int);

let findIndex = (predicate, array) =>
  try(
    {
      for (i in 0 to Array.length(array) - 1) {
        if (predicate(array[i])) {
          raise(Found(i));
        };
      };
      None;
    }
  ) {
  | Found(i) => Some(i)
  };

let slice = (~lines: array(_), ~start, ~length, ()) => {
  let len = Array.length(lines);
  if (start >= len) {
    [||];
  } else {
    let start = max(start, 0);
    let len = min(start + length, len) - start;
    if (len <= 0) {
      [||];
    } else {
      Array.sub(lines, start, len);
    };
  };
};

