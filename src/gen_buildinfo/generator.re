// This is a helper script that is run during build
// to produce [BuildInfo.re] - a generated module
// with context about the build environment & version.

let getCommitId = () => {
  let ic = Unix.open_process_in("git rev-parse --short HEAD");
  let commitId = input_line(ic);
  let () = close_in(ic);
  commitId;
};

let getBranch = () => {
  let ic = Unix.open_process_in("git rev-parse --abbrev-ref HEAD");
  let branch = input_line(ic) |> String.trim |> String.lowercase_ascii;
  let () = close_in(ic);
  branch;
};

let getDefaultUpdateChannel = () => {
  let currentBranch = getBranch();

  if (currentBranch == "staging" || currentBranch == "stable") {
    currentBranch;
  } else {
    "master";
  };
};

let getVersion = () => {
  Yojson.Safe.from_file("../../package.json")
  |> Yojson.Safe.Util.member("version")
  |> Yojson.Safe.Util.to_string;
};

let oc = open_out("BuildInfo.re");

Printf.fprintf(
  oc,
  {|
let commitId = "%s";
let version = "%s";
let defaultUpdateChannel = "%s";
|},
  getCommitId(),
  getVersion(),
  getDefaultUpdateChannel(),
);
close_out(oc);
