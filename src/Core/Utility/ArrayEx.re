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

let equals = (f, a1, a2) => {
  let len1 = Array.length(a1);
  let len2 = Array.length(a2);

  if (len1 != len2) {
    false;
  } else {
    let rec loop = idx =>
      if (idx >= len1) {
        true;
      } else if (!f(a1[idx], a2[idx])) {
        false;
      } else {
        loop(idx + 1);
      };
    loop(0);
  };
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
