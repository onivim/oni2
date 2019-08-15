open TestFramework;

module KeyDisplayer = Oni_Model.KeyDisplayer;

open KeyDisplayer;

describe("KeyDisplayer", ({describe, _}) => {
  describe("add", ({test, _}) => {
    test("keys typed close together get grouped", ({expect}) => {
      let v =
        KeyDisplayer.empty
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.1, "b");

      prerr_endline(KeyDisplayer.show(v));

      expect.bool(v == [{time: 1., keys: ["b", "a"]}]).toBe(true);
    });
    test("keys not close together don't get grouped", ({expect}) => {
      let v =
        KeyDisplayer.empty
        |> KeyDisplayer.add(1., "a")
        |> KeyDisplayer.add(1.3, "b");

      prerr_endline(KeyDisplayer.show(v));

      expect.bool(
        v == [{time: 1.3, keys: ["b"]}, {time: 1., keys: ["a"]}],
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
        |> KeyDisplayer.add(1.3, "b")
        |> KeyDisplayer.update(3.1);

      prerr_endline(KeyDisplayer.show(v));

      // "a" should be filtered out
      expect.bool(v == [{time: 1.3, keys: ["b"]}]).toBe(true);
    })
  );
});
