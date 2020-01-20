open EditorCoreTypes;
open Oni_Core;

module Protocol = ExtHostProtocol;

type t = {
  name: string,
  detail: string,
  kind: SymbolKind.t,
  //TODO: containerName?
  range: Range.t,
  //TODO: selectionRange?
  children: list(t),
};

let create = (~children=[], ~name, ~detail, ~kind, ~range) => {
  name,
  detail,
  kind,
  range,
  children,
};

let rec of_yojson_exn: Yojson.Safe.t => t =
  json => {
    Yojson.Safe.Util.(
      {
        let name = json |> member("name") |> to_string;
        let detail =
          json
          |> member("detail")
          |> to_string_option
          |> Oni_Core_Utility.Option.value(~default="");
        let range =
          json
          |> member("range")
          |> Protocol.OneBasedRange.of_yojson_exn
          |> Protocol.OneBasedRange.toRange;
        let children =
          json |> member("children") |> to_list |> List.map(of_yojson_exn);
        let kind = json |> member("kind") |> to_int |> SymbolKind.of_int;

        {name, detail, range, children, kind};
      }
    );
  };
