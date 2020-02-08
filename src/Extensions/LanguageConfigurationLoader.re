open Oni_Core;
open Oni_Core.Utility;

module Log = (val Log.withNamespace("Oni2.LanguageConfigurationLoader"));

type t = LazyLoader.t(LanguageConfiguration.t);

let create = languageInfo => {
  let loadFunction = language => {
    Log.debugf(m => m("Loading configuration for language: %s", language));
    language
    |> LanguageInfo.getLanguageConfigurationPath(languageInfo)
    |> Option.map(
         FunEx.tap(p => Log.debugf(m => m("Got path for language: %s", p))),
       )
    |> Option.to_result(
         ~none="No configuration defined for language: " ++ language,
       )
    |> Stdlib.Result.map(Yojson.Safe.from_file)
    |> (
      res2 =>
        Stdlib.Result.bind(res2, loadResult =>
          loadResult
          |> Json.Decode.decode_value(LanguageConfiguration.decode)
          |> Stdlib.Result.map_error(Json.Decode.string_of_error)
          |> Stdlib.Result.map_error(FunEx.tap(Log.error))
        )
    );
  };

  LazyLoader.create(loadFunction);
};

let get_opt = (languageConfigLoader, languageId) => {
  LazyLoader.get(languageConfigLoader, languageId) |> Stdlib.Result.to_option;
};
