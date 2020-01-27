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

      expect.bool(
        model.presses == [{time: 1.1, isExclusive: false, keys: ["b", "a"]}],
      ).
        toBe(
        true,
      );
    });
    test("keys not close together don't get grouped", ({expect}) => {
      let model =
        KeyDisplayer.initial
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.5, "b");

      expect.bool(
        model.presses
        == [
             {time: 1.5, isExclusive: false, keys: ["b"]},
             {time: 1., isExclusive: false, keys: ["a"]},
           ],
      ).
        toBe(
        true,
      );
    });
  });
  describe("update", ({test, _}) =>
    test("keys past the expiration time get filtered out", ({expect}) => {
      let model =
        KeyDisplayer.initial
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.5, "b")
        |> KeyDisplayer.update(3.9);

      // "a" should be filtered out
      expect.bool(
        model.presses == [{time: 1.5, isExclusive: false, keys: ["b"]}],
      ).
        toBe(
        true,
      );
    })
  );
});
