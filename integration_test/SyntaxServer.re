open Oni_CLI;

// Start syntax server
let argv = Sys.argv;
let (_options, eff) = Oni_CLI.parse(argv);
switch (eff) {
| StartSyntaxServer({parentPid, namedPipe}) =>
  Oni_Syntax_Server.start(~namedPipe, ~parentPid, ~healthCheck=()
    => 0) /* passed */
| _ => failwith("Expected syntax arguments")
};
