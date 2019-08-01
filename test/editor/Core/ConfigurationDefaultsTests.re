open TestFramework;

open Oni_Core.ConfigurationDefaults;

module Configuration = Oni_Core.Configuration;
module ConfigurationParser = Oni_Core.ConfigurationParser;

describe("ConfigurationDefaults", ({test, _}) => {
    test("configuration.json matches", ({expect}) => {
      let configJsonString =
        switch (getDefaultConfigString("configuration.json")) {
        | Some(c) => c
        | None => ""
        };

      switch(ConfigurationParser.ofString(configJsonString)) {
          | Error(_) => expect.int(1).toBe(2)
          | Ok(_) => expect.int(1).toBe(1)
      }
    });

    test("keybindings.json matches", ({expect}) => {
      let configJsonString =
        switch (getDefaultConfigString("keybindings.json")) {
        | Some(c) => c
        | None => ""
        };

      switch(ConfigurationParser.ofString(configJsonString)) {
          | Error(_) => expect.int(1).toBe(2)
          | Ok(_) => expect.int(1).toBe(1)
      }
    });
});
