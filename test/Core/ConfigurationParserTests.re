open TestFramework;

open Oni_Core.ConfigurationValues;

module Configuration = Oni_Core.Configuration;
module ConfigurationParser = Oni_Core.ConfigurationParser;
module Constants = Oni_Core.Constants;

describe("ConfigurationParser", ({test, describe, _}) => {
  describe("error handling", ({test, _}) => {
    test("invalid json returns error", ({expect, _}) => {
      let invalidConfiguration = "{]";

      switch (ConfigurationParser.ofString(invalidConfiguration)) {
      | Ok(_) => expect.string("Not OK").toEqual("")
      | Error(_) => expect.bool(true).toBe(true)
      };
    });

    test("json array returns error", ({expect, _}) => {
      let invalidConfiguration = "[]";

      switch (ConfigurationParser.ofString(invalidConfiguration)) {
      | Ok(_) => expect.string("Not OK").toEqual("")
      | Error(_) => expect.bool(true).toBe(true)
      };
    });
  });

  test("empty configuration", ({expect, _}) => {
    let emptyConfiguration = "{}";

    switch (ConfigurationParser.ofString(emptyConfiguration)) {
    | Ok(v) =>
      expect.string(Configuration.getValue(c => c.workbenchIconTheme, v)).
        toEqual(
        "vs-seti",
      )
    | Error(_) => expect.bool(false).toBe(true)
    };
  });

  test("vimUseSystemClipboard value", ({expect, _}) => {
    let configuration = {|
      { "vim.useSystemClipboard": [] }
      |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.bool(
        Configuration.getValue(c => c.vimUseSystemClipboard, v)
        == {yank: false, delete: false, paste: false},
      ).
        toBe(
        true,
      )
    | Error(_) => expect.bool(false).toBe(true)
    };

    let configuration = {|
      { "vim.useSystemClipboard": "yank" }
      |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.bool(
        Configuration.getValue(c => c.vimUseSystemClipboard, v)
        == {yank: true, delete: false, paste: false},
      ).
        toBe(
        true,
      )
    | Error(_) => expect.bool(false).toBe(true)
    };

    let configuration = {|
      { "vim.useSystemClipboard": ["yank", "delete"] }
      |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.bool(
        Configuration.getValue(c => c.vimUseSystemClipboard, v)
        == {yank: true, delete: true, paste: false},
      ).
        toBe(
        true,
      )
    | Error(_) => expect.bool(false).toBe(true)
    };

    let configuration = {|
      { "vim.useSystemClipboard": true }
      |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.bool(
        Configuration.getValue(c => c.vimUseSystemClipboard, v)
        == {yank: true, delete: true, paste: true},
      ).
        toBe(
        true,
      )
    | Error(_) => expect.bool(false).toBe(true)
    };
  });

  test("list of strings", ({expect, _}) => {
    let configuration = {|
     { "experimental.viml": ["first thing", "second thing", "third thing"] }
     |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.list(Configuration.getValue(c => c.experimentalVimL, v)).toEqual([
        "first thing",
        "second thing",
        "third thing",
      ])
    | Error(_) => expect.bool(false).toBe(true)
    };
  });
});
