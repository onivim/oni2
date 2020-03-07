open Oni_Core;
open TestFramework;

let uriSchemeStringJSON =
  {|
{"$mid":1,"external":"file:///Users/bryphe/oni2/test.css","path":"/Users/bryphe/oni2/test.css","scheme":"file"}
|}
  |> Yojson.Safe.from_string;

let uriSchemeArrayJSON =
  {|
{"$mid":1,"external":"file:///Users/bryphe/oni2/test.css","path":"/Users/bryphe/oni2/test.css","scheme":["memory"]}
|}
  |> Yojson.Safe.from_string;

let okOrFail = v =>
  switch (v) {
  | Ok(v) => v
  | Error(msg) => failwith(msg)
  };

describe("Uri", ({describe, _}) => {
  describe("toString", ({test, _}) => {
    test("adds slash for windows-style path", ({expect, _}) => {
      let uri = Uri.fromPath("C:/test");
      expect.string(Uri.toString(uri)).toEqual("file:///c:/test");
    })
  });
  describe("toFileSystemPath", ({test, _}) => {
    test("round-trip windows-style path", ({expect, _}) => {
      let uri = Uri.fromPath("C:/test");
      expect.string(Uri.toFileSystemPath(uri)).toEqual("c:/test");
    });
    test("round-trip posix path", ({expect, _}) => {
      let uri = Uri.fromPath("/users/onivim/test");
      expect.string(Uri.toFileSystemPath(uri)).toEqual(
        "/users/onivim/test",
      );
    });
  });
  describe("JSON", ({test, _}) => {
    test("parses with scheme as string", ({expect, _}) => {
      let scheme =
        Uri.of_yojson(uriSchemeStringJSON) |> okOrFail |> Uri.getScheme;

      expect.bool(scheme == Uri.Scheme.File).toBe(true);
    });
    test("parses with scheme as array", ({expect, _}) => {
      let scheme =
        Uri.of_yojson(uriSchemeArrayJSON) |> okOrFail |> Uri.getScheme;

      expect.bool(scheme == Uri.Scheme.Memory).toBe(true);
    });
  });
});
