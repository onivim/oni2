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

let getValue =
    (~fileType=None, selector: ConfigurationValues.t => 'a, configuration: t) => {
  let defaultValue = selector(configuration.default);

  switch (fileType) {
  | None => defaultValue
  | Some(f) =>
    switch (StringMap.find_opt(f, configuration.perFiletype)) {
    | None => defaultValue
    | Some(fileTypeConfig) => selector(fileTypeConfig)
    }
  };
};
