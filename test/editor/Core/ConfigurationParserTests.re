open TestFramework;

open Oni_Core.Configuration;

module ConfigurationParser = Oni_Core.ConfigurationParser;

describe("ConfigurationParser", ({test, describe, _}) => {
  describe("per-filetype handling", ({test, _}) => {
    test("simple filetype case", ({expect}) => {
      let fileTypeConfiguration = {|
      	"editor.insertSpaces": false,
		  "[reason]": {
		  	"editor.insertSpaces": true,
		  }
      |};

      switch (ConfigurationParser.ofString(fileTypeConfiguration)) {
      | Error(_) => expect.bool(true).toBe(false)
      | Ok(v) =>
        let insertSpaces =
          Configuration.getValue(
            ~fileType="reason",
            c => Some(c.editorInsertSpaces),
            v,
          );
        expect.bool(insertSpaces).toBe(true);

        let insertSpaces =
          Configuration.getValue(
            ~fileType="someotherlang",
            c => Some(c.editorInsertSpaces),
            v,
          );
        expect.bool(insertSpaces).toBe(false);
      };
    });

    test("ignores doubly-nested languages", ({expect}) => {
      let fileTypeConfiguration = {|
      	"editor.insertSpaces": false,
		  "[reason]": {
		  	"editor.insertSpaces": true,
			  "[ocaml]": {
			  	"editor.insertSpaces": false,
			  }
		  },
		  "[ocaml]": {
		  	"editor.insertSpaces": true
		  }
      |};
      ();
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
    | Ok(v) => expect.string(v.workbenchIconTheme).toEqual("vs-seti")
    | Error(_) => expect.bool(false).toBe(true)
    };
  });

  test("bool value", ({expect}) => {
    let configuration = {|
      { "editor.minimap.enabled": false }
      |};

    switch (ConfigurationParser.ofString(configuration)) {
    | Ok(v) => expect.bool(v.editorMinimapEnabled).toBe(false)
    | Error(_) => expect.bool(false).toBe(true)
    };
  });
});
