open TestFramework;

module ConfigurationParser = Oni_Core.ConfigurationParser;

describe("ConfigurationParser", ({test, _}) => {
  test("invalid configuration returns error", ({expect}) => {

      let invalidConfiguration = "{]";

      switch (ConfigurationParser.ofString(invalidConfiguration)) {
      | Ok(_) => expect.string("Not OK").toEqual("");
      | Error(_) => failwith("Config should be invalid");
      };
  });

  test("empty configuration", ({expect}) => {
      let emptyConfiguration = "{}";

      switch (ConfigurationParser.ofString(emptyConfiguration)) {
      | Ok(v) => {
          expect.string(v.workbenchIconTheme).toEqual("vs-seti");
      }
      | Error(_) => failwith("Config should be valid");
      };
  });
});
