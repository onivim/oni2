open TestFramework;

open Oni_Core.ConfigurationValues;

module Configuration = Oni_Core.Configuration;
module ConfigurationParser = Oni_Core.ConfigurationParser;
module Constants = Oni_Core.Constants;

describe("ConfigurationParser", ({test, describe, _}) => {
  describe("per-filetype handling", ({test, _}) => {
    test("simple filetype case", ({expect}) => {
      let fileTypeConfiguration = {|
		{
      	"editor.insertSpaces": false,
		  "[reason]": {
		  	"editor.insertSpaces": true
		  }
	  }
      |};

      switch (ConfigurationParser.ofString(fileTypeConfiguration)) {
      | Error(_) => expect.bool(true).toBe(false)
      | Ok(v) =>
        let insertSpaces =
          Configuration.getValue(
            ~fileType="reason",
            c => c.editorInsertSpaces,
            v,
          );
        expect.bool(insertSpaces).toBe(true);

        let insertSpaces =
          Configuration.getValue(
            ~fileType="someotherlang",
            c => c.editorInsertSpaces,
            v,
          );
        expect.bool(insertSpaces).toBe(false);
      };
    });

    test("ignores doubly-nested languages", ({expect}) => {
      let fileTypeConfiguration = {|
	  {
      	"editor.insertSpaces": false,
		  "[reason]": {
		  	"editor.insertSpaces": true,
			  "[ocaml]": {
			  	"editor.insertSpaces": false
			  }
		  },
		  "[ocaml]": {
		  	"editor.insertSpaces": true
		  }
	  }
      |};
      switch (ConfigurationParser.ofString(fileTypeConfiguration)) {
      | Error(_) => expect.int(1).toBe(2)
      | Ok(v) =>
        let ocamlInsertSpaces =
          Configuration.getValue(
            ~fileType="ocaml",
            c => c.editorInsertSpaces,
            v,
          );
        expect.bool(ocamlInsertSpaces).toBe(true);
      };
    });
  });
  describe("error handling", ({test, _}) => {
    test("invalid json returns error", ({expect}) => {
      let invalidConfiguration = "{]";

      switch (ConfigurationParser.ofString(invalidConfiguration)) {
      | Ok(_) => expect.string("Not OK").toEqual("")
      | Error(_) => expect.bool(true).toBe(true)
      };
    });

    test("json array returns error", ({expect}) => {
      let invalidConfiguration = "[]";

      switch (ConfigurationParser.ofString(invalidConfiguration)) {
      | Ok(_) => expect.string("Not OK").toEqual("")
      | Error(_) => expect.bool(true).toBe(true)
      };
    });
  });

  test("empty configuration", ({expect}) => {
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

  test("bool value", ({expect}) => {
    let configuration = {|
      { "editor.minimap.enabled": false }
      |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.bool(Configuration.getValue(c => c.editorMinimapEnabled, v)).
        toBe(
        false,
      )
    | Error(_) => expect.bool(false).toBe(true)
    };
  });

  let getExpectedValue = (valueGetter, configuration) => {
    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(parsedConfig) => Configuration.getValue(valueGetter, parsedConfig)
    | Error(_) => failwith("Unable to parse configuration: " ++ configuration)
    };
  };

  let getFontSize = getExpectedValue(c => c.editorFontSize);

  describe("editor.fontSize", ({test, _}) => {
    test("parses string if possible", ({expect}) => {
      let fontSize =
        getFontSize({|
        { "editor.fontSize": "12" }
      |});
      expect.float(fontSize).toBeCloseTo(12.);
    });

    test("uses default size if unable to parse", ({expect}) => {
      let fontSize =
        getFontSize({|
        { "editor.fontSize": "true" }
      |});
      expect.float(fontSize).toBeCloseTo(Constants.defaultFontSize);
    });

    test("does not allow value lower than minimum size", ({expect}) => {
      let fontSize =
        getFontSize({|
        { "editor.fontSize": 1 }
      |});
      expect.float(fontSize).toBeCloseTo(Constants.minimumFontSize);
    });
  });

  test("vimUseSystemClipboard value", ({expect}) => {
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

  test("list of numbers", ({expect}) => {
    let configuration = {|
      { "editor.rulers": [120, 80] }
    |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) =>
      expect.list(Configuration.getValue(c => c.editorRulers, v)).toEqual([
        80,
        120,
      ])
    | Error(_) => expect.bool(false).toBe(true)
    };
  });

  test("list of strings", ({expect}) => {
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

  test("resiliency tests", ({expect}) => {
    let trailingCommaInObject = {|
      { "editor.rulers": [120, 80], }
    |};

    let trailingCommaInArray = {|
      { "editor.rulers": [120, 80,] }
    |};

    let commentBeforeEverything = {|
      // This is my configuration
      { "editor.rulers": [120, 80] }
    |};

    let commentInObject = {|
      {
        // This is a setting
        "editor.rulers": [120, 80]
      }
    |};

    let cases = [
      trailingCommaInObject,
      trailingCommaInArray,
      commentBeforeEverything,
      commentInObject,
    ];

    List.iter(
      case => {
        switch (ConfigurationParser.ofString(case)) {
        | Ok(v) =>
          expect.list(Configuration.getValue(c => c.editorRulers, v)).toEqual([
            80,
            120,
          ])
        | Error(_) => expect.bool(false).toBe(true)
        }
      },
      cases,
    );
  });
});
