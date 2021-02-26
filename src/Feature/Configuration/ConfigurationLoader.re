open Oni_Core;
open Utility;

module Log = (
  val Oni_Core.Log.withNamespace(
        "Oni2.Feature.Configuration.ConfigurationLoader",
      )
);

let loadConfiguration = (loadResult: result(Yojson.Safe.t, string)) => {
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
      filePath: FpExp.t(FpExp.absolute),
      saveTick: int,
    });

let getFilePath: t => option(FpExp.t(FpExp.absolute)) =
  fun
  | None => None
  | File({filePath, _}) => Some(filePath);

let transformTask = (~transformer, model, ()) => {
  switch (model) {
  | None => ()
  | File({filePath, _}) =>
    let configPath = FpExp.toString(filePath);
    Oni_Core.Log.perf("Apply configuration transform", () => {
      let parsedJson = Yojson.Safe.from_file(configPath);
      let newJson = transformer(parsedJson);
      let oc = open_out(configPath);
      Yojson.Safe.pretty_to_channel(oc, newJson);
      close_out(oc);
    });
  };
};

let none = None;

let file = filePath => File({filePath, saveTick: 0});

let notifyFileSaved = path =>
  fun
  | None => None
  | File({filePath, saveTick}) as orig =>
    if (FpExp.eq(filePath, path)) {
      File({filePath, saveTick: saveTick + 1});
    } else {
      orig;
    };

let reload =
  fun
  | None => None
  | File({saveTick, _} as orig) => File({...orig, saveTick: saveTick + 1});

let sub =
  fun
  | None => Isolinear.Sub.none
  | File({saveTick, filePath, _}) => File.sub(~filePath, ~saveTick);

let loadImmediate =
  fun
  | None => Error("No file to load")
  | File({filePath, _}) => {
      filePath |> FpExp.toString |> JsonEx.from_file |> loadConfiguration;
    };
