open Oni_Core;

type t = LazyLoader.t(LanguageConfiguration.t);

let create = (languageInfo) => {

	let loadFunction = (language) => {
		language
		|> LanguageInfo.getLanguageConfigurationPath(languageInfo)
		|> Option.to_result(~none="No configuration defined for language: " ++ language)
		|> Stdlib.Result.map(Yojson.Safe.from_file)
		|> Stdlib.Result.bind(Json.Decode.decode_value(LanguageConfiguration.decode));
	};

	LazyLoader.create(loadFunction);
};

let get_opt = (languageConfigLoader, languageId) => {
	LazyLoader.get(languageConfigLoader, languageId)
	|> Result.to_option;
};
