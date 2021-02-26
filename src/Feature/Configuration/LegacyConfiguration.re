/*
 * LegacyConfiguration.re
 *
 * Configuration settings for the editor
 *
 * This is the DEPRECATED way of working with configuration - we should not add to this, and actively work towards refactoring it out.
 */
open Oni_Core;

type t = {
  default: LegacyConfigurationValues.t,
  perFiletype: StringMap.t(LegacyConfigurationValues.t),
};

let default = {
  default: LegacyConfigurationValues.default,
  perFiletype: StringMap.empty,
};

let getValue =
    (
      ~fileType=?,
      selector: LegacyConfigurationValues.t => 'a,
      configuration: t,
    ) => {
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
