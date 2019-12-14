open EditorCoreTypes;
open Oni_Core;

type t = {
  uri: Uri.t,
  range: Range.t,
};

let create = (~uri, ~range) => { uri, range };

let of_yojson_exn = json => {
  Yojson.Safe.Util.({
    let uri = json |> member("uri") |> Uri.of_yojson |> Utility.resultToException;
    let range = json |> member("range") 
      |> ExtHostProtocol.OneBasedRange.of_yojson_exn
      |> ExtHostProtocol.OneBasedRange.toRange;
    {uri, range};
  });
}

