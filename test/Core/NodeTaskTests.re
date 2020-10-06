open Oni_Core;
open Utility;
open TestFramework;

let setup = Setup.init();

describe("NodeTask", ({test, _}) => {
  test("successful task", ({expect, _}) => {
    let result = NodeTask.run(~setup, "success.js") |> LwtEx.sync;
    expect.equal(result, Ok("Successful node task!"));
  });

  test("task with error", ({expect, _}) => {
    let result = NodeTask.run(~setup, "error.js") |> LwtEx.sync;
    expect.equal(result |> Result.is_error, true);
  });
});
