open EditorCoreTypes;
open Oni_Core;

type t = {
  uri: Uri.t,
  range: Range.t,
};

let create = (~uri, ~range) => { uri, range };

let of_yojson_exn = json => {
  Yojson.Safe.Util.(
    let uri = json |> member |> Uri.of_yojson |> Utility.resultToException;
    let range = json |> member |> ExtHostProtocol.OneBasedRange.of_yojson_exn;
    {uri, range};
  );
}

