open TestFramework;
open Vim;

let reset = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));

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
});
