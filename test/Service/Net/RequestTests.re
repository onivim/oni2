open Oni_Core;
open Oni_Core.Utility;
open TestFramework;

open Service_Net;

let setup = Oni_Core.Setup.init();

describe("Request", ({describe, _}) => {
  describe("json", ({test, _}) => {
    test("success", ({expect, _}) => {
      let response =
        Request.json(
          ~setup,
          ~decoder=Json.Decode.value,
          "https://httpbin.org/json",
        )
        |> LwtEx.sync;

      expect.equal(Result.is_ok(response), true);
    });

    test("fail: 400", ({expect, _}) => {
      let response =
        Request.json(
          ~setup,
          ~decoder=Json.Decode.value,
          "https://httpbin.org/status/404",
        )
        |> LwtEx.sync;

      expect.equal(Result.is_error(response), true);
    });

    test("fail: 500", ({expect, _}) => {
      let response =
        Request.json(
          ~setup,
          ~decoder=Json.Decode.value,
          "https://httpbin.org/status/500",
        )
        |> LwtEx.sync;

      expect.equal(Result.is_error(response), true);
    });
  });
  describe("download", ({test, _}) => {
    test("no path specified", ({expect, _}) => {
      let downloadPath =
        Request.download(~setup, "https://httpbin.org/image/jpeg")
        |> LwtEx.sync;

      expect.equal(Result.is_ok(downloadPath), true);

      // Check the file
      let filePath = Result.get_ok(downloadPath);
      let stat: Luv.File.Stat.t =
        Luv.File.Sync.stat(filePath) |> Result.get_ok;

      let size = stat.size |> Unsigned.UInt64.to_int;

      // Validate there are at least some bytes written...
      expect.equal(size == 35588, true);
    })
  });
});
