// Not very robust path-handling utilities.
// TODO: Make good

let hasTrailingSeparator = path => {
  let len = String.length(path);
  if (len == 0) {
    false;
  } else {
    let last = path.[len - 1];
    last == '\\' || last == '/';
  };
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

let getExtension = path => {
  let fileName = try (Rench.Path.filename(path)) {
    | Invalid_argument(_) => ""
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
     let%test "No file name, only extension" = getExtension("/home/oni/.bashrc") == ".bashrc";
     let%test "No path" = getExtension("") == "";
   });
