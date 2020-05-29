module Primitives = {
  open SimpleXml;
  open Plist;

  let rec value =
    fun
    | Element("true", []) => True
    | Element("false", []) => False
    | Element("string", [Text(str)]) => String(str)
    | Element("string", []) => String("")
    | Element("integer", [Text(str)]) => Integer(int_of_string(str))
    | Element("real", [Text(str)]) => Real(float_of_string(str))
    | Element("array", items) => Array(List.map(value, items))
    | Element("dict", proplist) => Dict(properties(proplist))
    | other =>
      failwith("invalid or unsupported value: " ++ SimpleXml.show(other))

  and properties =
    fun
    | [] => []
    | [Element("key", [Text(key)]), valueElement, ...rest] => [
        (key, value(valueElement)),
        ...properties(rest),
      ]
    | other => {
        let fragment =
          other |> List.map(SimpleXml.show) |> String.concat("\n");
        failwith("invalid property list: \n" ++ fragment);
      };

  let plist =
    fun
    | Element("plist", [Element("dict", proplist)]) =>
      Dict(properties(proplist))
    | other => failwith("not plist: " ++ SimpleXml.show(other));
};

let parse = xml =>
  switch (Primitives.plist(xml)) {
  | plist => Ok(plist)
  | exception (Failure(message)) => Error(message)
  };
