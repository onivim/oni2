type t = {
  name: string,
  executable: string,
  arguments: list(string),
};

let to_yojson = ({name, executable, arguments}) => {
  let args = arguments |> List.map(s => `String(s));
  `Assoc([
    ("name", `String(name)),
    ("executable", `String(executable)),
    ("args", `List(args)),
  ]);
};
