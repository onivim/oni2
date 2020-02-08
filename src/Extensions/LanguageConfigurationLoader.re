open Oni_Core;

type t = LazyLoader.t(LanguageConfiguration.t);

let create = languageInfo => {
  let loadFunction = language => {
    language
    |> LanguageInfo.getLanguageConfigurationPath(languageInfo)
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
        )
    );
  };

  LazyLoader.create(loadFunction);
};

let get_opt = (languageConfigLoader, languageId) => {
  LazyLoader.get(languageConfigLoader, languageId) |> Stdlib.Result.to_option;
};
