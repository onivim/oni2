/* open Oni_Core; */
open TestFramework;

/* open Helpers; */

module IconTheme = Oni_Model.IconTheme;

let testTheme = {|
    {
	"information_for_contributors": [""],
	"fonts": [
		{
			"id": "seti",
			"src": [
				{
					"path": "./seti.ttf",
					"format": "woff"
				}
			],
			"weight": "normal",
			"style": "normal",
			"size": "150%"
		}
	],
	"iconDefinitions": {
		"_default": {
			"fontCharacter": "\\E001",
			"fontColor": "#ff0000"
		},
		"_test_ext": {
			"fontCharacter": "\\E002",
			"fontColor": "#6d8086"
		},
		"_test_file": {
			"fontCharacter": "\\E090",
			"fontColor": "#6d8086"
		},
		"_test_language": {
			"fontCharacter": "\\E091",
			"fontColor": "#6d8086"
		}
	},
	"file": "_default",
	"fileExtensions": {
		"ext": "_test_ext"
	},
	"fileNames": {
		"file1": "_test_file"
	},
	"languageIds": {
		"language1": "_test_language"
	},
	"version": ""
}
|};



let json: Yojson.Safe.json = Yojson.Safe.from_string(testTheme);

describe("IconTheme", ({test, _}) =>{
    test("gets icon for matching filename", ({expect}) => {
        let _iconTheme = IconTheme.ofJson(json) |> Oni_Core_Test.Helpers.getOrThrow;
        let icon: option(IconTheme.IconDefinition.t) = IconTheme.getIconForFile(_iconTheme, "file1", "some-random-language");

        switch(icon) {
        | Some(v) => expect.int(v.fontCharacter).toBe(0xE090)
        | None => expect.string("No icon found!").toEqual("")
        }
    });

    test("gets icon for matching extension", ({expect}) => {
        let _iconTheme = IconTheme.ofJson(json) |> Oni_Core_Test.Helpers.getOrThrow;
        let icon: option(IconTheme.IconDefinition.t) = IconTheme.getIconForFile(_iconTheme, "file1.ext", "some-random-language");

        switch(icon) {
        | Some(v) => expect.int(v.fontCharacter).toBe(0xE002)
        | None => expect.string("No icon found!").toEqual("")
        }
    });

    test("gets icon for matching extension", ({expect}) => {
        let _iconTheme = IconTheme.ofJson(json) |> Oni_Core_Test.Helpers.getOrThrow;
        let icon: option(IconTheme.IconDefinition.t) = IconTheme.getIconForFile(_iconTheme, "file1.rnd", "language1");

        switch(icon) {
        | Some(v) => expect.int(v.fontCharacter).toBe(0xE091)
        | None => expect.string("No icon found!").toEqual("")
        }
    });

    test("falls back to default icon", ({expect}) => {
        let _iconTheme = IconTheme.ofJson(json) |> Oni_Core_Test.Helpers.getOrThrow;
        let icon: option(IconTheme.IconDefinition.t) = IconTheme.getIconForFile(_iconTheme, "file1.rnd", "unknown-language");

        switch(icon) {
        | Some(v) => expect.int(v.fontCharacter).toBe(0xE001)
        | None => expect.string("No icon found!").toEqual("")
        }
    });

    });
