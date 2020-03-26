open Rely.MatcherTypes;

open Kernel;

// TREE EXTENSIONS

type treeExtensions = {toEqual: KeyedStringTree.t(int) => unit};

let treeExtensions = (actual, {createMatcher}) => {
  let pass = (() => "", true);
  let fail = message => (() => message, false);

  let rec toString =
    fun
    | KeyedStringTree.Leaf(value) => Printf.sprintf("Leaf(%n)", value)
    | KeyedStringTree.Node(children) => {
        let children =
          children
          |> KeyedStringTree.KeyedMap.to_seq
          |> Seq.map(((key, value)) =>
               Printf.sprintf("(\"%s\", %s)", key, toString(value))
             )
          |> List.of_seq;

        Printf.sprintf("Node([%s])", String.concat(", ", children));
      };

  let rec treeEqual =
          (actual: KeyedStringTree.t(_), expected: KeyedStringTree.t(_)) =>
    switch (actual, expected) {
    | (Leaf(a), Leaf(b)) when a == b => true
    | (Node(aChildren), Node(bChildren)) =>
      Base.List.equal(
        ((aKey, aChild), (bKey, bChild)) =>
          aKey == bKey && treeEqual(aChild, bChild),
        KeyedStringTree.KeyedMap.bindings(aChildren),
        KeyedStringTree.KeyedMap.bindings(bChildren),
      )

    | _ => false
    };

  let createTreeMatcher =
    createMatcher(
      ({formatReceived, formatExpected, _}, actualThunk, expectedThunk) => {
      let actual = actualThunk();
      let expected = expectedThunk();

      if (treeEqual(actual, expected)) {
        pass;
      } else {
        let failureMessage =
          Printf.sprintf(
            "Expected: %s\nReceived: %s",
            formatExpected(toString(expected)),
            formatReceived(toString(actual)),
          );
        fail(failureMessage);
      };
    });

  {toEqual: expected => createTreeMatcher(() => actual, () => expected)};
};

// CUSTOM MATCHERS

type customMatchers = {tree: KeyedStringTree.t(int) => treeExtensions};

let customMatchers = createMatcher => {
  tree: actual => treeExtensions(actual, createMatcher),
};

// INIT

include Rely.Make({
  let config =
    Rely.TestFrameworkConfig.initialize({
      snapshotDir: "__snapshots__",
      projectDir: "",
    });
});

let {describe, describeOnly, describeSkip} =
  describeConfig |> withCustomMatchers(customMatchers) |> build;
