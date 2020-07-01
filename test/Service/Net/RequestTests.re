open Oni_Core;
open Oni_Core.Utility;
open TestFramework;

open Service_Net;

let setup = Oni_Core.Setup.init();

describe("Request", ({test, _}) => {
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
