// Not very robust path-handling utilities.
// TODO: Make good

let toRelative = (~base, path) => {
  let base = base == "/" ? base : base ++ Filename.dir_sep;
  Str.replace_first(Str.regexp_string(base), "", path);
};

let explode = String.split_on_char(Filename.dir_sep.[0]);

let join = paths => {
  let sep = Filename.dir_sep;
  let (head, rest) =
    switch (paths) {
    | [] => ("", [])
    | [head, ...rest] => (head, rest)
    };
  List.fold_left((accum, p) => accum ++ sep ++ p, head, rest);
};

let trimTrailingSeparator = path => {
  let len = String.length(path);
  let last = path.[len - 1];

  /* Remove trailing slashes */
  if (last == '\\' || last == '/') {
    String.sub(path, 0, len - 1);
  } else {
    path;
  };
};