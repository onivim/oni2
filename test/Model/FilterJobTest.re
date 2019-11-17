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
      command: () =>
        Oni_Model.Actions.ShowNotification(
          Oni_Model.Notification.create(~title="derp", ~message=name, ()),
        ),
      icon: None,
      highlight: [],
    };

  let runToCompletion = j => {
    let job = ref(j);

    while (!Job.isComplete(job^)) {
      job := Job.tick(job^);
    };

    job^;
  };

  describe("filtering", ({test, _}) => {
    test("filtering should respect smart casing", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.addItems([createItem("Preferences")]))
        |> Job.map(FilterJob.updateQuery("pref"))
        |> runToCompletion;

      expect.int(List.length(Job.getCompletedWork(job).uiFiltered)).toBe(
        1,
      );
    });
    test("updating query should not reset items", ({expect, _}) => {
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
        |> Job.doWork
        |> Job.map(FilterJob.updateQuery("abce"));

      // We should have results without needing to do another iteration of work
      let filtered = Job.getCompletedWork(job).allFiltered;
      expect.int(List.length(filtered)).toBe(1);

      let head = List.hd(filtered);

      expect.string(head.name).toEqual("abcde");
    });
    test("items batched separately get filtered", ({expect, _}) => {
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

      let filtered = Job.getCompletedWork(job).allFiltered;
      expect.int(List.length(filtered)).toBe(2);

      let head = List.hd(filtered);
      let second = List.nth(filtered, 1);

      expect.string(head.name).toEqual("abcd");
      expect.string(second.name).toEqual("abcde");
    });
    test("items batched together get filtered", ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.updateQuery("abc"))
        // Add 4 items, separately
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

      let filtered = Job.getCompletedWork(job).allFiltered;
      expect.int(List.length(filtered)).toBe(2);

      let head = List.hd(filtered);
      let second = List.nth(filtered, 1);

      expect.string(head.name).toEqual("abcde");
      expect.string(second.name).toEqual("abcd");
    });

    test(
      "regresion test - already filterd items shouldn't get re-added",
      ({expect, _}) => {
      let job =
        FilterJob.create()
        |> Job.map(FilterJob.addItems([createItem("abcd")]))
        |> Job.map(FilterJob.updateQuery("a"))
        |> Job.tick
        |> Job.map(FilterJob.addItems([createItem("a")]))
        |> Job.map(FilterJob.updateQuery("abc"))
        |> runToCompletion;

      let filtered = Job.getCompletedWork(job).allFiltered;
      expect.int(List.length(filtered)).toBe(1);

      let head = List.hd(filtered);

      expect.string(head.name).toEqual("abcd");
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
