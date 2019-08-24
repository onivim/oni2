open TestFramework;

open Oni_Core;

module MenuJob = Oni_Model.MenuJob;
module Actions = Oni_Model.Actions;

describe("MenuJob", ({describe, _}) => {
  let createItem = name => {
    let ret: Actions.menuCommand = {
      category: None,
      name,
      command: () =>
        Oni_Model.Actions.ShowNotification(
          Oni_Model.Notification.create(~title="derp", ~message=name, ()),
        ),
      icon: None,
    };
    ret;
  };

  describe("filtering", ({test, _}) => {
    test("items batched separately get filtered", ({expect, _}) => {
      let job =
        MenuJob.create()
        |> Job.map(MenuJob.updateQuery("abc"))
        // Add 4 items, separately
        |> Job.map(MenuJob.addItems([createItem("a")]))
        |> Job.map(MenuJob.addItems([createItem("abcd")]))
        |> Job.map(MenuJob.addItems([createItem("b")]))
        |> Job.map(MenuJob.addItems([createItem("abcde")]))
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
        MenuJob.create()
        |> Job.map(MenuJob.updateQuery("abc"))
        // Add 4 items, separately
        |> Job.map(
             MenuJob.addItems([
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
  });
  describe("boundary cases", ({test, _}) =>
    test("large amount of items added work", ({expect, _}) => {
      let job = MenuJob.create();

      let commands: list(Actions.menuCommand) =
        List.init(1000000, i => createItem("Item " ++ string_of_int(i)));

      let job = Job.map(MenuJob.addItems(commands), job);

      expect.bool(Job.isComplete(job)).toBe(false);
    })
  );
});
