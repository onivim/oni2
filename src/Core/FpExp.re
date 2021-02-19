// Experimental utilities on top of Fp -
// could potentially be merged into Fp when stable

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

let absolutePlatform = (~fromPlatform, str) => {
  str |> Fp.absolutePlatform(~fromPlatform);
};

let absoluteCurrentPlatform = str => {
  switch (Revery.Environment.os) {
  | Revery.Environment.Windows(_) =>
    str
    |> Fp.absolutePlatform(~fromPlatform=Fp.Windows(Win32))
    |> Option.map(fp => Path(fp))
  | _ =>
    str
    |> Fp.absolutePlatform(~fromPlatform=Fp.Posix)
    |> Option.map(fp => Path(fp))
  };
};

// TODO: Fix for UNC
let toString =
  fun
  | Path(path) => Fp.toString(path)
  | UNC({path, _}) =>
    // TODO
    Fp.toString(path);

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
    Fp.relativize(source, dest)
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
