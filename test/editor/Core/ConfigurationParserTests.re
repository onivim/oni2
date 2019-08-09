open TestFramework;

open Oni_Core.ConfigurationValues;

module Configuration = Oni_Core.Configuration;
module ConfigurationParser = Oni_Core.ConfigurationParser;

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
});
