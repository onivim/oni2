// Not very robust path-handling utilities.
// TODO: Replace with a strongly-typed solution like `fp`:
// https://github.com/facebookexperimental/reason-native/tree/master/src/fp

module Log = (val Kernel.Log.withNamespace("Oni2.Core.Utility.Path"));

let hasTrailingSeparator = path => {
  let len = String.length(path);
  if (len == 0) {
    false;
  } else {
    let last = path.[len - 1];
    last == '\\' || last == '/';
  };
};

let normalizeBackSlashes = {
  let backSlashRegex = Str.regexp("\\\\");
  path => Str.global_replace(backSlashRegex, "/", path);
};

let toRelative = (~base, path) => {
  let base = hasTrailingSeparator(base) ? base : base ++ Filename.dir_sep;
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

  /* Remove trailing slashes */
  if (hasTrailingSeparator(path)) {
    String.sub(path, 0, len - 1);
  } else {
    path;
  };
};

let filename = path =>
  try(Rench.Path.filename(path)) {
  | Invalid_argument(_) =>
    Log.warnf(m => m("getExtension - invalid filename: %s", path));
    "";
  };

let dirname = path =>
  try(Rench.Path.dirname(path)) {
  | Invalid_argument(_) =>
    Log.warnf(m => m("getExtension - invalid basename: %s", path));
    "";
  };

let getExtension = path => {
  let fileName =
    try(Rench.Path.filename(path)) {
    | Invalid_argument(_) =>
      Log.warnf(m => m("getExtension - invalid filename: %s", path));
      "";
    };

  if (String.length(fileName) == 0) {
    "";
  } else if (fileName.[0] == '.') {
    fileName;
  } else {
    Rench.Path.extname(fileName);
  };
};

let%test_module "getExtension" =
  (module
   {
     let%test "Simple file" = getExtension("/home/oni/test.md") == ".md";
     let%test "Simple file, no extension" =
       getExtension("/home/oni/test") == "";
     let%test "No file name, only extension" =
       getExtension("/home/oni/.bashrc") == ".bashrc";
     let%test "No path" = getExtension("") == "";
   });
