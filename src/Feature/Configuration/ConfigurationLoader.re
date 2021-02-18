open Oni_Core;
open Utility;

module Log = (
  val Oni_Core.Log.withNamespace(
        "Oni2.Feature.Configuration.ConfigurationLoader",
      )
);

let loadConfiguration = (loadResult: result(Yojson.Safe.t, string)) => {
  // let parseResult =
  // loadResult
  //   |> Utility.ResultEx.flatMap(Keybindings.of_yojson_with_errors);
  // switch (parseResult) {
  // | Ok((bindings, errors)) => (bindings, errors)
  // | Error(msg) => ([], [msg])
  //};
  loadResult
  |> ResultEx.flatMap(json => {
       let legacyConfigParseResult = LegacyConfigurationParser.ofJson(json);
       legacyConfigParseResult
       |> Result.map(legacyConfig => (json, legacyConfig));
     })
  |> Result.map(((json, legacyConfig)) => {
       let config = Config.Settings.fromJson(json);
       (config, legacyConfig);
     });
};

module File = {
  let sub = (~filePath, ~saveTick) => {
    Oni_Core.SubEx.jsonFile(
      ~uniqueId="Feature_Configuration.ConfigurationLoader",
      ~filePath,
      ~tick=saveTick,
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

let loadImmediate =
  fun
  | None => Error("No file to load")
  | File({filePath, _}) => {
      filePath |> Fp.toString |> JsonEx.from_file |> loadConfiguration;
    };
