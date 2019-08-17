open TestFramework;

module KeyDisplayer = Oni_Model.KeyDisplayer;

open KeyDisplayer;

describe("KeyDisplayer", ({describe, _}) => {
  describe("add", ({test, _}) => {
    test("keys typed close together get grouped", ({expect}) => {
      let v =
        KeyDisplayer.empty
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.1, "b")
        |> KeyDisplayer.getPresses;

      expect.bool(v == [{time: 1., exclusive: false, keys: ["b", "a"]}]).
        toBe(
        true,
      );
    });
    test("keys not close together don't get grouped", ({expect}) => {
      let v =
        KeyDisplayer.empty
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.5, "b")
        |> KeyDisplayer.getPresses;

      expect.bool(
        v
        == [
             {time: 1.5, exclusive: false, keys: ["b"]},
             {time: 1., exclusive: false, keys: ["a"]},
           ],
      ).
        toBe(
        true,
      );
    });
  });
  describe("update", ({test, _}) =>
    test("keys past the expiration time get filtered out", ({expect}) => {
      let v =
        KeyDisplayer.empty
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.5, "b")
        |> KeyDisplayer.update(5.1)
        |> KeyDisplayer.getPresses;

      // "a" should be filtered out
      expect.bool(v == [{time: 1.5, exclusive: false, keys: ["b"]}]).toBe(
        true,
      );
    })
  );
});
