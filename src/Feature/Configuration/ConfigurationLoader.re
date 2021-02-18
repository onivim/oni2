module Log = (
  val Oni_Core.Log.withNamespace("Oni2.Feature.Configuration.ConfigurationLoader")
);

module File = {
  let loadConfiguration = (loadResult: result(Yojson.Safe.t, string)) => {
    // let parseResult =
    // loadResult
    //   |> Utility.ResultEx.flatMap(Keybindings.of_yojson_with_errors);

    // switch (parseResult) {
    // | Ok((bindings, errors)) => (bindings, errors)
    // | Error(msg) => ([], [msg])
    //};
    loadResult
  };

  let sub = (~filePath, ~saveTick) => {
    Oni_Core.SubEx.jsonFile(
      ~uniqueId="Feature_Configuration.ConfigurationLoader",
      ~filePath,
      ~tick=saveTick
    )
    |> Isolinear.Sub.map(loadConfiguration);
  };
};

type t =
  | None
  | File({
      filePath: Fp.t(Fp.absolute),
      saveTick: int,
    });

let none = None;

let file = filePath => File({filePath, saveTick: 0});

let notifyFileSaved = path =>
  fun
  | None => None
  | File({filePath, saveTick}) as orig =>
    if (Fp.eq(filePath, path)) {
      File({filePath, saveTick: saveTick + 1});
    } else {
      orig;
    };

let sub =
  fun
  | None => Isolinear.Sub.none
  | File({saveTick, filePath}) => File.sub(~filePath, ~saveTick);
