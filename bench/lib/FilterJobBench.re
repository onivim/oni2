open Oni_Core;
open Oni_Model;
open BenchFramework;

module MenuFilterJob =
  FilterJob.Make({
    type item = Actions.menuItem;
    let format = Quickmenu.getLabel;
  });

let createItem = name => {
  let ret: Actions.menuItem = {
    category: None,
    name,
    command: _ => Oni_Model.Actions.Noop,
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

let smallerNumberOfItems =
  List.init(10000, i =>
    createItem(
      "Some item with a long name but with index " ++ string_of_int(i),
    )
  );

let getJob = items =>
  MenuFilterJob.create()
  |> Job.map(MenuFilterJob.updateQuery("item 1"))
  |> Job.map(MenuFilterJob.addItems(items))
  |> Job.map(MenuFilterJob.addItems(items))
  |> Job.map(MenuFilterJob.addItems(items))
  |> Job.map(MenuFilterJob.addItems(items))
  |> Job.map(MenuFilterJob.addItems(items));

let job = getJob(largeAmountOfItems);
let runJob = () => Job.doWork(job) |> (ignore: Job.t(_) => unit);

let completeJob = () => {
  let job = ref(getJob(smallerNumberOfItems));
  while (!Job.isComplete(job^)) {
    job := Job.doWork(job^);
  };
};

let setup = () => ();

let options = Reperf.Options.create(~iterations=1000, ());

bench(~name="FilterJob: doWork atom", ~setup, ~options, ~f=runJob, ());

bench(
  ~name="FilterJob: addItems",
  ~setup,
  ~options,
  ~f=
    () => {
      let _: Job.t(_) =
        MenuFilterJob.create()
        |> Job.map(MenuFilterJob.addItems(largeAmountOfItems));
      ();
    },
  (),
);

let options = Reperf.Options.create(~iterations=100, ());
bench(~name="FilterJob: completeJob", ~setup, ~options, ~f=completeJob, ());
