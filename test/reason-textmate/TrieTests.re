open TestFramework;

module Trie = Textmate.Trie;

describe("Trie", ({describe, _}) =>
  describe("matches", ({test, _}) => {
    test("matches empty tree returns empty list", ({expect, _}) => {
      let ret = Trie.matches(Trie.empty, ["scope"]);
      expect.bool(ret == []).toBe(true);
    });

    test("single item matches", ({expect, _}) => {
      let update = _ => Some(2);

      let trie = Trie.update(["scope"], update, Trie.empty);

      let ret = Trie.matches(trie, ["scope"]);

      expect.bool(ret == [("scope", Some(2))]).toBe(true);
    });
    test("multiple item matches", ({expect, _}) => {
      let update1 = _ => Some(1);
      let update2 = _ => Some(2);

      let trie =
        Trie.empty
        |> Trie.update(["scope"], update1)
        |> Trie.update(["scope", "js"], update2);

      let ret = Trie.matches(trie, ["scope", "js"]);

      expect.bool(ret == [("js", Some(2)), ("scope", Some(1))]).toBe(
        true,
      );
    });
    test("partial match", ({expect, _}) => {
      let update1 = _ => Some(1);
      let update2 = _ => Some(2);

      let trie =
        Trie.empty
        |> Trie.update(["scope"], update1)
        |> Trie.update(["scope", "js"], update2);

      let ret = Trie.matches(trie, ["scope", "js", "extra"]);

      expect.bool(ret == [("js", Some(2)), ("scope", Some(1))]).toBe(
        true,
      );
    });
  })
);
