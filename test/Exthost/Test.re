open Exthost_Extension;

open Exthost;
open Exthost_TestLib;
open Oni_Core;
open Oni_Core.Utility;

module Log = (val Timber.Log.withNamespace("Test"));

module InitData = Extension.InitData;

type t = {
  client: Client.t,
  extensionProcess: Luv.Process.t,
  processHasExited: ref(bool),
  messages: ref(list(Msg.t)),
};

let noopHandler = _ => Lwt.return(Reply.okEmpty);
let noopErrorHandler = _ => ();

let getExtensionManifest =
    (
      ~rootPath=Rench.Path.join(Sys.getcwd(), "test/collateral/extensions"),
      name,
    ) => {
  name
  |> Rench.Path.join(rootPath)
  |> (
    p =>
      Rench.Path.join(p, "package.json")
      |> Scanner.load(~category=Scanner.Bundled)
      |> Option.map(InitData.Extension.ofScanResult)
      |> Option.get
  );
};

let startWithExtensions =
    (
      ~rootPath=Rench.Path.join(Sys.getcwd(), "test/collateral/extensions"),
      ~initialConfiguration=Exthost.Configuration.empty,
      ~pid=Luv.Pid.getpid(),
      ~handler=noopHandler,
      ~onError=noopErrorHandler,
      extensions,
    ) => {
  Log.info("Starting test!");
  let messages = ref([]);

  let errorHandler = err => {
    prerr_endline("ERROR: " ++ err);
    onError(err);
  };

  let wrappedHandler = msg => {
    prerr_endline("Received msg: " ++ Msg.show(msg));
    messages := [msg, ...messages^];
    handler(msg);
  };

  Timber.App.enable(Timber.Reporter.console());
  Oni_Core.Log.init();

  let extensions =
    extensions
    |> List.map(Rench.Path.join(rootPath))
    |> List.map(p => Rench.Path.join(p, "package.json"))
    |> List.map(Scanner.load(~category=Scanner.Bundled))
    |> List.filter_map(v => v)
    |> List.map(InitData.Extension.ofScanResult);

  extensions |> List.iter(m => m |> InitData.Extension.show |> prerr_endline);

  let logFile = Filename.temp_file("test", "log") |> Uri.fromPath;
  let logsLocation = Filename.get_temp_dir_name() |> Uri.fromPath;

  Log.errorf(m =>
    m(
      "Log location: %s Log file: %s",
      logFile |> Uri.toString,
      logsLocation |> Uri.toString,
    )
  );

  let parentPid = pid;

  let initData =
    InitData.create(
      ~version="9.9.9",
      ~parentPid,
      ~logLevel=1, // DEBUG
      ~logsLocation,
      ~logFile,
      extensions,
    );
  let pipe = NamedPipe.create("pipe-for-test");
  let pipeStr = NamedPipe.toString(pipe);
  let client =
    Client.start(
      ~initialConfiguration,
      ~namedPipe=pipe,
      ~initData,
      ~handler=wrappedHandler,
      ~onError=errorHandler,
      (),
    )
    |> ResultEx.tapError(msg => prerr_endline(msg))
    |> Result.get_ok;

  let processHasExited = ref(false);

  let onExit = (_, ~exit_status: int64, ~term_signal: int) => {
    prerr_endline(
      Printf.sprintf(
        "Process exited: %d signal: %d",
        Int64.to_int(exit_status),
        term_signal,
      ),
    );
    processHasExited := true;
  };

  let extHostScriptPath = Setup.getNodeExtensionHostPath(Setup.init());

  let extensionProcess =
    Node.spawn(
      ~env=[
        ("PATH", Oni_Core.ShellUtility.getPathFromEnvironment()),
        (
          "VSCODE_AMD_ENTRYPOINT",
          "vs/workbench/services/extensions/node/extensionHostProcess",
        ),
        ("PIPE_LOGGING", "true"), // Pipe logging to parent
        ("VERBOSE_LOGGING", "true"), // Pipe logging to parent
        ("VSCODE_IPC_HOOK_EXTHOST", pipeStr),
        ("VSCODE_PARENT_PID", parentPid |> string_of_int),
      ],
      ~onExit,
      [extHostScriptPath],
    );
  {client, extensionProcess, processHasExited, messages};
};

let close = context => {
  Client.close(context.client);
  context;
};

let activateByEvent = (~event, context) => {
  Request.ExtensionService.activateByEvent(~event, context.client)
  |> Utility.LwtEx.sync
  |> Result.get_ok;
  context;
};

let executeContributedCommand = (~command, context) => {
  Request.Commands.executeContributedCommand(
    ~arguments=[],
    ~command,
    context.client,
  );
  context;
};

let fail = (~name, msg) => {
  prerr_endline(Printf.sprintf("== CONDITION %s FAILED: %s", name, msg));
  exit(2);
};

let waitForProcessClosed = ({processHasExited, _}) => {
  Waiter.wait(
    ~onFail=fail, ~timeout=15.0, ~name="Wait for node process to close", () =>
    processHasExited^
  );
};

let waitForMessage = (~name, f, {messages, _} as context) => {
  Waiter.wait(~onFail=fail, ~name="Wait for message: " ++ name, () =>
    List.exists(f, messages^)
  );

  context;
};
let waitForReady = context => {
  waitForMessage(~name="Ready", msg => msg == Ready, context);
};

let waitForInitialized = context => {
  waitForMessage(~name="Initialized", msg => msg == Initialized, context);
};

let waitForExtensionActivation = (expectedExtensionId, context) => {
  let waitForActivation =
    fun
    | Msg.ExtensionService(DidActivateExtension({extensionId, _})) =>
      String.equal(extensionId, expectedExtensionId)
    | _ => false;

  context
  |> waitForMessage(
       ~name="Activation:" ++ expectedExtensionId,
       waitForActivation,
     );
};

let withClient = (f, context) => {
  f(context.client);
  context;
};

let withClientRequest = (~name, ~validate, f, context) => {
  let response = f(context.client);
  let hasValidated = ref(false);

  let validator = returnValue => {
    if (!validate(returnValue)) {
      fail(~name, "Validation failed");
    };
    hasValidated := true;
  };
  let () = Lwt.on_success(response, validator);
  Waiter.wait(~onFail=fail, ~timeout=10.0, ~name="Waiter: " ++ name, () => {
    hasValidated^
  });

  context;
};

let activate = (~extensionId, ~reason, context) => {
  context
  |> withClientRequest(
       ~name="Activating extension: " ++ extensionId,
       ~validate=v => v,
       Exthost.Request.ExtensionService.activate(~extensionId, ~reason),
     );
};

let validateNoPendingRequests = context => {
  if (Client.Testing.getPendingRequestCount(context.client) > 0) {
    fail(~name="Pending Requests", "There are still pending requests");
  };
  context;
};

let terminate = context => {
  Client.terminate(context.client);
  context;
};
