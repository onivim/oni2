Console.log("Running FileWatcherTest...");

let changed = ref(false);

// Create test file
let tempFilePath = Filename.temp_file("test", ".txt");
let oc = open_out(tempFilePath);
Printf.fprintf(oc, "foo");
close_out(oc);

// System under test
let sub =
  Service_FileWatcher.watch(~path=tempFilePath, ~onEvent=_ => changed := true);

module Runner =
  Isolinear.Testing.SubscriptionRunner.Make({
    type msg = unit;
  });

Runner.run(~dispatch=() => (), ~sub, Runner.empty);

// Modify file on disk
let oc = open_out(tempFilePath);
Printf.fprintf(oc, "bar");
close_out(oc);

// Runner
let idler = Luv.Idle.init() |> Result.get_ok;
Luv.Idle.start(idler, () =>
  if (changed^) {
    Console.log("Success!");
    Luv.Loop.stop(Luv.Loop.default());
  }
)
|> Result.get_ok;

let timer = Luv.Timer.init() |> Result.get_ok;
Luv.Timer.start(timer, 1000, () => failwith("Timeout")) |> Result.get_ok;

ignore(Luv.Loop.run(): bool);
