open TestFramework;
open Oni_Core;

module Actions = Oni_Model.Actions;
module FilterJob =
  Oni_Model.FilterJob.Make({
    type item = Actions.menuItem;
    let format = Oni_Model.Quickmenu.getLabel;
  });

describe("FilterJob", ({describe, _}) => {
  let createItem = name =>
    Actions.{
      category: None,
      name,
      command: _ => Actions.Noop,
      icon: None,
      highlight: [],
    };

  let rec runToCompletion = job =>
    Job.isComplete(job) ? job : job |> Job.tick |> runToCompletion;

  let getNames = ranked =>
    List.map(
      (result: Filter.result(Actions.menuItem)) => result.item.name,
      ranked,
    );

  describe("ranking", ({test, _}) => {
    test("ranking should respect smart casing", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.addItems([createItem("Preferences")]))
        |> Job.map(FilterJob.updateQuery("pref"))
        |> runToCompletion;

      let rankedLength = List.length(Job.getCompletedWork(job).ranked);
      expect.int(rankedLength).toBe(1);
    });

    test("items batched separately get ranked", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.updateQuery("abc"))
        // Add 4 items, separately
        |> Job.map(FilterJob.addItems([createItem("a")]))
        |> Job.map(FilterJob.addItems([createItem("abcd")]))
        |> Job.map(FilterJob.addItems([createItem("b")]))
        |> Job.map(FilterJob.addItems([createItem("abcde")]))
        // Tick 4 times
        |> Job.doWork
        |> Job.doWork
        |> Job.doWork
        |> Job.doWork;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["abcd", "abcde"]);
    });

    test("items batched together get ranked", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.updateQuery("abc"))
        // Add 4 items, together
        |> Job.map(
             FilterJob.addItems([
               createItem("a"),
               createItem("abcd"),
               createItem("b"),
               createItem("abcde"),
             ]),
           )
        // Tick 4 times
        |> Job.doWork
        |> Job.doWork
        |> Job.doWork
        |> Job.doWork;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["abcd", "abcde"]);
    });

    test("items returned are deterministic", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.updateQuery("ab"))
        // Add 4 items, together
        |> Job.map(
             FilterJob.addItems([
               createItem("aba"),
               createItem("abb"),
               createItem("abc"),
               createItem("abd"),
             ]),
           )
        // Tick 4 times
        |> runToCompletion;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["aba", "abb", "abc", "abd"]);
    });

    test(
      "should reset when updating query with a broader filter", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(
             FilterJob.addItems([createItem("abc"), createItem("abde")]),
           )
        |> Job.map(FilterJob.updateQuery("abc"))
        |> runToCompletion;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["abc"]);

      let job =
        job |> Job.map(FilterJob.updateQuery("abd")) |> runToCompletion;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["abde"]);

      let job =
        job |> Job.map(FilterJob.updateQuery("ab")) |> runToCompletion;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["abc", "abde"]);
    });

    test(
      "regression test - already ranked items shouldn't get re-added",
      ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.addItems([createItem("abcd")]))
        |> Job.map(FilterJob.updateQuery("a"))
        |> Job.tick
        |> Job.map(FilterJob.addItems([createItem("a")]))
        |> Job.map(FilterJob.updateQuery("abc"))
        |> runToCompletion;

      let ranked = Job.getCompletedWork(job).ranked;
      let names = getNames(ranked);
      expect.list(names).toEqual(["abcd"]);
    });
  });

  describe("boundary cases", ({test, _}) =>
    test("large amount of items added work", ({expect, _}) => {
      let job = FilterJob.create();

      let items: list(Actions.menuItem) =
        List.init(1000000, i => createItem("Item " ++ string_of_int(i)));

      let job = Job.map(FilterJob.addItems(items), job);
      expect.bool(Job.isComplete(job)).toBe(false);
    })
  );
});
