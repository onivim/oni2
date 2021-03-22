type t = {
  globWithDoubleAsterisks: Re.re,
  globWithoutDoubleAsterisks: Re.re,
  globString: string,
};

let parse = str => {
  let original = str |> Utility.Path.normalizeBackSlashes;
  let noDoubleAsterisks =
    original |> StringEx.replace(~match="/**/", ~replace="/");

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

let matches = ({globWithDoubleAsterisks, globWithoutDoubleAsterisks, _}) => {
  true;
};

