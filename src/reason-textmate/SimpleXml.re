[@deriving show({with_path: false})]
type t =
  | Element(string, list(t))
  | Text(string);

let trim =
  Markup.filter(
    fun
    | `Text(strs) when strs |> String.concat("") |> String.trim == "" =>
      false
    | _ => true,
  );

let simplify = stream =>
  stream
  |> trim
  |> Markup.tree(
       ~text=strings => Text(strings |> String.concat("")),
       ~element=((_ns, name), _attrs, children) => Element(name, children),
     );

let of_file = path =>
  Markup.file(path) |> fst |> Markup.parse_xml |> Markup.signals |> simplify;
