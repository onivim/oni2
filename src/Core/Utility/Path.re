// Not very robust path-handling utilities.
// TODO: Make good

let toRelative = (~base, path) => {
  let base = base == "/" ? base : base ++ Filename.dir_sep;
  Str.replace_first(Str.regexp_string(base), "", path);
};

let explode = String.split_on_char(Filename.dir_sep.[0]);
