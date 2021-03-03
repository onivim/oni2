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

let filterToList = (f, array) => {
  Array.fold_left(
    (acc, cur) =>
      if (f(cur)) {
        [cur, ...acc];
      } else {
        acc;
      },
    [],
    array,
  );
};

let replace = (~replacement, ~start, ~stop, arr) =>
  if (Array.length(arr) == 0) {
    replacement;
  } else if (start >= Array.length(arr)) {
    Array.concat([arr, replacement]);
  } else {
    // Get beginning of array
    let prev = slice(~lines=arr, ~start=0, ~length=start, ());

    let post =
      slice(~lines=arr, ~start=stop, ~length=Array.length(arr) - stop, ());

    Array.concat([prev, replacement, post]);
  };
