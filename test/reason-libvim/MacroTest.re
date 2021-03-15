open TestFramework;
open Vim;

let reset = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

let isStartRecording =
  fun
  | Effect.MacroRecordingStarted(_) => true
  | _ => false;

let isStopRecording =
  fun
  | Effect.MacroRecordingStopped(_) => true
  | _ => false;

let isMacroEffect = eff => isStartRecording(eff) || isStopRecording(eff);

describe("Macro", ({test, _}) => {
  test("recording dispatches start", ({expect, _}) => {
    let _ = reset();

    let effects = ref([]);
    let dispose =
      onEffect(eff =>
        effects := [eff, ...effects^] |> List.filter(isMacroEffect)
      );

    input("q");

    expect.int(List.length(effects^)).toBe(0);

    input("a");

    expect.int(List.length(effects^)).toBe(1);

    expect.equal(effects^, [Effect.MacroRecordingStarted({register: 'a'})]);

    input("q");

    dispose();
  });
  test("stop recording - empty", ({expect, _}) => {
    let _ = reset();

    let effects = ref([]);
    let dispose =
      onEffect(eff =>
        effects := [eff, ...effects^] |> List.filter(isStopRecording)
      );

    input("q");

    expect.int(List.length(effects^)).toBe(0);

    input("a");

    input("q");

    expect.int(List.length(effects^)).toBe(1);

    expect.equal(
      effects^,
      [Effect.MacroRecordingStopped({register: 'a', value: Some("")})],
    );

    dispose();
  });
  test("stop recording - register value", ({expect, _}) => {
    let _ = reset();

    let effects = ref([]);
    let dispose =
      onEffect(eff =>
        effects := [eff, ...effects^] |> List.filter(isStopRecording)
      );

    input("q");

    expect.int(List.length(effects^)).toBe(0);

    input("a");

    input("j");

    input("q");

    expect.int(List.length(effects^)).toBe(1);

    expect.equal(
      effects^,
      [Effect.MacroRecordingStopped({register: 'a', value: Some("j")})],
    );

    dispose();
  });
  test("regression test for #934: macro using global command", ({expect, _}) => {
    let buf = reset();

    Vim.Buffer.setLines(
      ~shouldAdjustCursors=false,
      ~lines=[|
        {|<meta name="”robots”" content="”noindex,nofollow,noarchive,nosnippet,noodp”">|},
        {|<meta name="apple-mobile-web-app-capable" content="yes">|},
        {|<meta name="viewport" content="width=device-width, user-scalable=no,initial-scale=1.0,maximum-scale=1.0">|},
        {|<meta name="format-detection" content="telephone=no">|},
        {|<meta name="format-detection" content="address=no">|},
        "",
      |],
      buf,
    );

    // Record macro
    ["q", "q", "^", "w", "c", "i", "w", "test", "<Esc>", "q"]
    |> List.iter(key);

    let (_: Vim.Context.t, _: list(Vim.Effect.t)) =
      Vim.command("g/meta/normal @q");

    let actualLines = Vim.Buffer.getLines(buf);
    let expectedLines = [|
      {|<test name="”robots”" content="”noindex,nofollow,noarchive,nosnippet,noodp”">|},
      {|<test name="apple-mobile-web-app-capable" content="yes">|},
      {|<test name="viewport" content="width=device-width, user-scalable=no,initial-scale=1.0,maximum-scale=1.0">|},
      {|<test name="format-detection" content="telephone=no">|},
      {|<test name="format-detection" content="address=no">|},
      "",
    |];

    expect.equal(actualLines, expectedLines);
  });
});
