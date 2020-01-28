open TestFramework;

module KeyDisplayer = Oni_Model.KeyDisplayer;

open KeyDisplayer;

describe("KeyDisplayer", ({describe, _}) => {
  describe("add", ({test, _}) => {
    test("keys typed close together get grouped", ({expect}) => {
      let model =
        KeyDisplayer.initial
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.1, "b");

      expect.list([{time: 1.1, isExclusive: false, keys: ["b", "a"]}]).
        toEqual(
        model.presses,
      );
    });
    test("keys not close together don't get grouped", ({expect}) => {
      let model =
        KeyDisplayer.initial
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.5, "b");

      expect.list([
        {time: 1.5, isExclusive: false, keys: ["b"]},
        {time: 1., isExclusive: false, keys: ["a"]},
      ]).
        toEqual(
        model.presses,
      );
    });
  });
  describe("update", ({test, _}) =>
    test("keys past the expiration time get filtered out", ({expect}) => {
      let model =
        KeyDisplayer.initial
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.5, "b")
        |> KeyDisplayer.add(3.9, "x");

      // "a" should be filtered out
      expect.list([
        {time: 3.9, isExclusive: false, keys: ["x"]},
        {time: 1.5, isExclusive: false, keys: ["b"]},
      ]).
        toEqual(
        model.presses,
      );
    })
  );
});
