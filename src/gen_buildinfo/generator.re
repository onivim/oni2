// This is a helper script that is run during build
// to produce [BuildInfo.re] - a generated module
// with context about the build environment & version.

let getCommitId = () => {
  let ic = Unix.open_process_in("git rev-parse --short HEAD");
  let commitId = input_line(ic);
  let () = close_in(ic);
  commitId;
};

let getVersion = () => {
  Yojson.Safe.from_file("../../package.json")
  |> Yojson.Safe.Util.member("version")
  |> Yojson.Safe.Util.to_string;
};

let getExtensionHostVersion = () => {
  Yojson.Safe.from_file(
    "../../../../../../../../node/node_modules/@onivim/vscode-exthost/package.json",
  )
  |> Yojson.Safe.Util.member("version")
  |> Yojson.Safe.Util.to_string;
};

let oc = open_out("BuildInfo.re");

Printf.fprintf(
  oc,
  {|
let commitId = "%s";
let version = "%s";
let extensionHostVersion = "%s";
|},
  getCommitId(),
  getVersion(),
  getExtensionHostVersion(),
);
close_out(oc);
