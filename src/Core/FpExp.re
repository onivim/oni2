// Experimental utilities on top of Fp -
// could potentially be merged into Fp when stable

open Utility;

type relative = Fp.relative;
type absolute = Fp.absolute;

type platform = Fp.platform;

type t('kind) =
  | UNC({
      server: string,
      share: string,
      path: Fp.t(absolute),
    })
    : t(absolute)
  | Path(Fp.t('kind)): t('kind);

module UNCParser = {
  type t = {
    server: string,
    share: string,
    path: string,
  };

  let parse = str => {
    let len = String.length(str);

    if (len <= 3) {
      None;
    } else if (str.[0] == '\\' && str.[1] == '\\') {
      // At least the start of a unc path...
      let remainder = String.sub(str, 2, len - 2);

      switch (String.split_on_char('\\', remainder)) {
      | [] => None
      | [server, share, ...remainingElements] =>
        Some({server, share, path: String.concat("\\", remainingElements)})
      | _ => None
      };
    } else {
      None;
    };
  };

  let%test_module "UNC Parser" =
    (module
     {
       let cases = [
         ("c:\\temp\\test-file.txt", None),
         (
           "\\\\127.0.0.1\\c$\\temp\\test-file.txt",
           Some({
             server: "127.0.0.1",
             share: "c$",
             path: "temp\\test-file.txt",
           }),
         ),
         // "\\\\LOCALHOST\\c$\\temp\\test-file.txt",
         // "\\\\.\\c:\\temp\\test-file.txt",
         // "\\\\?\\c:\\temp\\test-file.txt",
         // "\\\\.\\UNC\\LOCALHOST\\c$\\temp\\test-file.txt",
         // "\\\\127.0.0.1\\c$\\temp\\test-file.txt",
       ];
       let%test "unc paths should parse when platform is windows" = {
         cases
         |> List.for_all(((path, expected)) => parse(path) == expected);
       };
     });
};

let absolutePlatform = (~fromPlatform, str) =>
  // If we're on Windows - try parsing as a UNC path
  if (fromPlatform == Fp.Windows(Win32)) {
    str
    |> UNCParser.parse
    |> OptionEx.flatMap(({server, share, path}: UNCParser.t) => {
         let fpPath = Fp.absolutePlatform(~fromPlatform, "\\" ++ path);
         fpPath |> Option.map(path => {UNC({path, server, share})});
       })
    // If not a UNC path... switch back to
    |> OptionEx.or_lazy(() =>
         str
         |> Fp.absolutePlatform(~fromPlatform)
         |> Option.map(path => Path(path))
       );
  } else {
    str
    |> Fp.absolutePlatform(~fromPlatform)
    |> Option.map(path => Path(path));
  };

let root = Path(Fp.root);

let absoluteCurrentPlatform = str => {
  switch (Revery.Environment.os) {
  | Revery.Environment.Windows(_) =>
    str |> absolutePlatform(~fromPlatform=Fp.Windows(Win32))
  | _ => str |> absolutePlatform(~fromPlatform=Fp.Posix)
  };
};

// TODO: Fix for UNC
let toString =
  fun
  | Path(path) => Fp.toString(path)
  | UNC({path, server, share}) =>
    Printf.sprintf(
      "\\\\%s\\%s%s",
      server,
      share,
      Fp.toString(path) |> String.split_on_char('/') |> String.concat("\\"),
    );

let%test_module "UNC Paths" =
  (module
   {
     let fromPlatform = Fp.Windows(Win32);
     let cases = [
       "\\\\127.0.0.1\\c$\\temp\\test-file.txt",
       "\\\\LOCALHOST\\c$\\temp\\test-file.txt",
       "\\\\.\\c:\\temp\\test-file.txt",
       "\\\\?\\c:\\temp\\test-file.txt",
       "\\\\.\\UNC\\LOCALHOST\\c$\\temp\\test-file.txt",
       "\\\\127.0.0.1\\c$\\temp\\test-file.txt",
     ];
     let%test "unc paths should parse when platform is windows" = {
       cases
       |> List.for_all(path =>
            absolutePlatform(~fromPlatform, path) |> Option.is_some
          );
     };

     let%test "unc paths round-trip with current brackets on windows" = {
       cases
       |> List.for_all(path => {
            let actual =
              absolutePlatform(~fromPlatform, path) |> Option.get |> toString;
            String.equal(path, actual);
          });
     };
   });

let isDescendent = (~ofPath, path) => {
  switch (ofPath, path) {
  | (UNC({path: ofPath, _}), UNC({path, _})) =>
    Fp.isDescendent(~ofPath, path)
  | (Path(ofPath), Path(path)) => Fp.isDescendent(~ofPath, path)
  | _ => false
  };
};

let relativize = (~source, ~dest) => {
  switch (source, dest) {
  | (UNC({path: source, _}), UNC({path: dest, _})) =>
    Fp.relativize(~source, ~dest)
  | (Path(source), Path(dest)) => Fp.relativize(~source, ~dest)
  | _ => Error(Invalid_argument("Not matching path types"))
  };
};

let baseName =
  fun
  | UNC({path, _})
  | Path(path) => Fp.baseName(path);

let dirName =
  fun
  | UNC({path, server, share}) =>
    UNC({server, share, path: Fp.dirName(path)})
  | Path(path) => Path(Fp.dirName(path));

let eq = (pathA, pathB) =>
  switch (pathA, pathB) {
  | (
      UNC({path: pathA, server: serverA, share: shareA}),
      UNC({path: pathB, server: serverB, share: shareB}),
    ) =>
    Fp.eq(pathA, pathB)
    && String.equal(serverA, serverB)
    && String.equal(shareA, shareB)
  | (Path(pathA), Path(pathB)) => Fp.eq(pathA, pathB)
  | _ => false
  };

module At = {
  let (/) = (path, name) =>
    switch (path) {
    | UNC({path, server, share}) =>
      UNC({server, share, path: Fp.At.(path / name)})
    | Path(path) => Path(Fp.At.(path / name))
    };
};

let hasParentDir =
  fun
  | UNC({path, _}) => Fp.hasParentDir(path)
  | Path(path) => Fp.hasParentDir(path);

let append = At.(/);
