[@deriving show]
type t = {
  globWithDoubleAsterisks: [@opaque] Re.re,
  globWithoutDoubleAsterisks: [@opaque] Re.re,
  globString: string,
};

let toDebugString = ({globString, _}) => globString;

let parse = str => {
  let original = str |> Utility.Path.normalizeBackSlashes;
  let noDoubleAsterisks =
    original |> Utility.StringEx.replace(~match="/**/", ~replace="/");

  try({
    let globWithDoubleAsterisks =
      original |> Re.Glob.glob(~expand_braces=true) |> Re.compile;

    let globWithoutDoubleAsterisks =
      noDoubleAsterisks |> Re.Glob.glob(~expand_braces=true) |> Re.compile;
    Ok({
      globWithDoubleAsterisks,
      globWithoutDoubleAsterisks,
      globString: original,
    });
  }) {
  | exn => Error(Printexc.to_string(exn))
  };
};

let matches =
    ({globWithDoubleAsterisks, globWithoutDoubleAsterisks, _}, path) => {
  Re.matches(globWithDoubleAsterisks, path) != []
  || Re.matches(globWithoutDoubleAsterisks, path) != [];
};

let decode =
  Json.Decode.(
    {
      string
      |> map(parse)
      |> and_then(
           fun
           | Ok(glob) => succeed(glob)
           | Error(msg) => fail(msg),
         );
    }
  );

let%test_module "Glob" =
  (module
   {
     let%test "double asterisk matches directory" = {
       let glob = parse("/abc/**/*.hs") |> Result.get_ok;

       matches(glob, "/abc/src/file.hs");
     };

     let%test "double asterisk matches file at root, too" = {
       let glob = parse("/abc/**/*.hs") |> Result.get_ok;

       matches(glob, "/abc/file.hs");
     };
   });
