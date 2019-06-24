/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

[@deriving show({with_path: false})]
type t = {
  default: ConfigurationValues.t,
  perFiletype: StringMap.t(ConfigurationValues.t)
};

let default = {
  default: ConfigurationValues.t,
  perFiletype: StringMap.empty,
};

let getValueOpt = (selector: (ConfigurationValues.t) => option('a), fileType: option(string) = None, configuration: t) => {

	let defaultValue = selector(configuration.default);

	let ret = switch (fileType) {
	| None => defaultValue
	| Some(f) => {
		switch(StringMap.find_opt(configuration.perFiletype, f)) {
		| None => defaultValue
		| Some(filetypeConfig) => switch(selector(fileTypeConfig)) {
		| None => defaultValue
		| Some(c) => c
		}
		}
	}
	}
};

exception ConfigurationNotFound;

let getValue = (selector: (ConfigurationValues.t) => option('a), fileType: option(string) = None, configuration: t) => {
	switch (getValueOpt(selector, fileType, configuration)) {
	| Some(v) => v
	| None => raise(ConfigurationNotFound)
	}
};
