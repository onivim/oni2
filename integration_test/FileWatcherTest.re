// Disable colors on windows to prevent hanging on CI

Timber.App.enable(Timber.Reporter.console());
Timber.App.setLevel(Timber.Level.trace);

print_endline("Running FileWatcherTest...");

// Create test file
let tempFilePath = Filename.temp_file("test", ".txt");
let oc = open_out(tempFilePath);
Printf.fprintf(oc, "foo");
close_out(oc);

// System under test
let sub =
  Service_FileWatcher.watch(
    ~path=tempFilePath,
    ~onEvent=_ => {
      print_endline("Success!");
      Luv.Loop.stop(Luv.Loop.default());
    },
  );

module Runner =
  Isolinear.Testing.SubscriptionRunner.Make({
    type msg = unit;
  });

Runner.run(~dispatch=() => (), ~sub, Runner.empty);

// Modify file on disk
let timer = Luv.Timer.init() |> Result.get_ok;
Luv.Timer.start(
  timer,
  100,
  () => {
    let oc = open_out(tempFilePath);
    Printf.fprintf(oc, "bar");
    close_out(oc);
  },
)
|> Result.get_ok;

// Runner
let timer = Luv.Timer.init() |> Result.get_ok;
Luv.Timer.start(timer, 10000, () => failwith("Timeout")) |> Result.get_ok;

ignore(Luv.Loop.run(): bool);
