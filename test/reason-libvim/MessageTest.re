open TestFramework;
open Vim;

let reset = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Messages", ({describe, test, _}) => {
  describe("ex cmds", ({test, _}) => {
    test(":messages produces Goto Messages effect", ({expect, _}) => {
      let _ = reset();

      let (_: Context.t, effects) = command("messages");

      expect.equal(effects, [Vim.(Effect.Goto(Goto.Messages))]);
    });
    test(":messages clear produces clear effect with 0 count", ({expect, _}) => {
      let _ = reset();

      let (_: Context.t, effects) = command("messages clear");

      expect.equal(
        effects,
        [Vim.(Effect.Clear(Clear.{target: Messages, count: 0}))],
      );
    });
    test(
      ":5messages clear produces clear effect with 5 count", ({expect, _}) => {
      let _ = reset();

      let (_: Context.t, effects) = command("5messages clear");

      expect.equal(
        effects,
        [Vim.(Effect.Clear(Clear.{target: Messages, count: 5}))],
      );
    });
  });
  test("echo dispatches message", ({expect, _}) => {
    let _ = reset();

    let messages = ref([]);
    let dispose =
      onMessage((priority, title, contents) =>
        messages := [(priority, title, contents), ...messages^]
      );

    let (_: Context.t, _: list(Effect.t)) = command("echo 'hello'");

    expect.int(List.length(messages^)).toBe(1);

    let (priority, title, contents) = List.hd(messages^);

    expect.string(title).toEqual("");
    expect.string(contents).toEqual("hello");
    expect.bool(priority == Types.Info).toBe(true);

    dispose();
  });
  test("echoerr dispatches error message", ({expect, _}) => {
    let _ = reset();

    let messages = ref([]);
    let dispose =
      onMessage((priority, title, contents) =>
        messages := [(priority, title, contents), ...messages^]
      );

    let (_: Context.t, _: list(Effect.t)) = command("echoerr 'aproblem'");

    /* TODO: Fix this! */
    /* expect.int(List.length(messages^)).toBe(1); */

    let (priority, title, contents) = List.hd(messages^);

    expect.string(title).toEqual("");
    expect.string(contents).toEqual("aproblem");
    expect.bool(priority == Types.Error).toBe(true);

    dispose();
  });
});
