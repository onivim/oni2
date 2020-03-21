open TestFramework;

open Oni_Core.ConfigurationDefaults;

module Configuration = Oni_Core.Configuration;
module ConfigurationParser = Oni_Core.ConfigurationParser;

describe("ConfigurationDefaults", ({test, _}) =>
  test("configuration.json matches", ({expect, _}) => {
    let configJsonString =
      switch (getDefaultConfigString("configuration.json")) {
      | Some(c) => c
      | None => ""
      };

    let defaultConfig = Oni_Core.Configuration.default;

    switch (ConfigurationParser.ofString(configJsonString)) {
    | Error(_) => expect.int(1).toBe(2)
    | Ok(loadedConfig) => expect.equal(defaultConfig, loadedConfig)
    };
  })
);
