/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

type t = {
  default: ConfigurationValues.t,
  perFiletype: StringMap.t(ConfigurationValues.t),
};

let default = {
  default: ConfigurationValues.default,
  perFiletype: StringMap.empty,
};

let getValueOpt =
    (
      ~fileType=None,
      selector: ConfigurationValues.t => option('a),
      configuration: t,
    ) => {
  let defaultValue = selector(configuration.default);

  switch (fileType) {
  | None => defaultValue
  | Some(f) =>
    switch (StringMap.find_opt(f, configuration.perFiletype)) {
    | None => defaultValue
    | Some(fileTypeConfig) =>
      switch (selector(fileTypeConfig)) {
      | None => defaultValue
      | Some(c) => Some(c)
      }
    }
  };
};

exception ConfigurationNotFound;

let getValue =
    (
      ~fileType=None,
      selector: ConfigurationValues.t => option('a),
      configuration: t,
    ) => {
  switch (getValueOpt(~fileType, selector, configuration)) {
  | Some(v) => v
  | None => raise(ConfigurationNotFound)
  };
};
