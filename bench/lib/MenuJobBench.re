open Oni_Core;
open Oni_Model;
open BenchFramework;

let createItem = name => {
  let ret: Actions.menuCommand = {
    category: None,
    name,
    command: () =>
      Oni_Model.Actions.ShowNotification(
        Oni_Model.Notification.create(~title="derp", ~message=name, ()),
      ),
    icon: None,
    highlight: [],
  };
  ret;
};

let largeAmountOfItems =
  List.init(1000000, i =>
    createItem(
      "Some item with a long name but with index " ++ string_of_int(i),
    )
  );

let job =
  MenuJob.create()
  |> Job.map(MenuJob.updateQuery("item 1"))
  |> Job.map(MenuJob.addItems(largeAmountOfItems))
  |> Job.map(MenuJob.addItems(largeAmountOfItems))
  |> Job.map(MenuJob.addItems(largeAmountOfItems))
  |> Job.map(MenuJob.addItems(largeAmountOfItems))
  |> Job.map(MenuJob.addItems(largeAmountOfItems));

let runJob = () => {
  let _ = Job.doWork(job);
  ();
};

let setup = () => ();

let options = Reperf.Options.create(~iterations=1000, ());

bench(~name="MenuJob: doWork atom", ~setup, ~options, ~f=runJob, ());
