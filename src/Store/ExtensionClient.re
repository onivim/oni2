open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStore"));

let create =
    (~initialWorkspace, ~attachStdio, ~config, ~extensions, ~setup: Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  Log.infof(m =>
    m("ExtensionClient.create called with attachStdio: %b", attachStdio)
  );

  let maybeClientRef = ref(None);

  let extensionInfo =
    extensions |> List.map(Exthost.Extension.InitData.Extension.ofScanResult);
  open Exthost;
  open Exthost.Extension;
  open Exthost.Msg;

  let handler: Msg.t => Lwt.t(Reply.t) =
    msg => {
      switch (msg) {
      | Initialized =>
        dispatch(Actions.Exthost(Feature_Exthost.Msg.initialized));
        Lwt.return(Reply.okEmpty);
      | DownloadService(msg) => Middleware.download(msg)

      | FileSystem(msg) =>
        let (promise, resolver) = Lwt.task();

        let fileSystemMsg = Feature_FileSystem.Msg.exthost(~resolver, msg);
        dispatch(FileSystem(fileSystemMsg));
        promise;

      | SCM(msg) =>
        Feature_SCM.handleExtensionMessage(
          ~dispatch=msg => dispatch(Actions.SCM(msg)),
          msg,
        );
        Lwt.return(Reply.okEmpty);

      | Storage(msg) =>
        let (promise, resolver) = Lwt.task();

        let storageMsg = Feature_Extensions.Msg.storage(~resolver, msg);
        dispatch(Extensions(storageMsg));

        promise;

      | Commands(ExecuteCommand({command, args, _})) =>
        // TODO: Is this really the right action?
        dispatch(Actions.CommandInvoked({command, arguments: `List(args)}));
        Lwt.return(Reply.okEmpty);

      | Configuration(msg) =>
        dispatch(
          Actions.Configuration(Feature_Configuration.Msg.exthost(msg)),
        );
        Lwt.return(Reply.okEmpty);

      | Diagnostics(diagnosticMsg) =>
        dispatch(
          Actions.Diagnostics(
            Feature_Diagnostics.Msg.exthost(diagnosticMsg),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | DocumentContentProvider(documentContentProviderMsg) =>
        dispatch(
          Actions.SCM(
            Feature_SCM.Msg.documentContentProvider(
              documentContentProviderMsg,
            ),
          ),
        );

        Lwt.return(Reply.okEmpty);

      | Decorations(decorationsMsg) =>
        dispatch(
          Decorations(Feature_Decorations.Msg.exthost(decorationsMsg)),
        );
        Lwt.return(Reply.okEmpty);

      | Documents(documentsMsg) =>
        let (promise, resolver) = Lwt.task();
        dispatch(
          Actions.Exthost(
            Feature_Exthost.Msg.document(documentsMsg, resolver),
          ),
        );
        promise;

      | ExtensionService(extMsg) =>
        Log.infof(m => m("ExtensionService: %s", Exthost.Msg.show(msg)));
        dispatch(
          Actions.Extensions(Feature_Extensions.Msg.exthost(extMsg)),
        );
        Lwt.return(Reply.okEmpty);

      | Languages(msg) =>
        let (promise, resolver) = Lwt.task();

        let languagesMsg = Feature_Extensions.Msg.languages(~resolver, msg);
        dispatch(Extensions(languagesMsg));

        promise;

      | LanguageFeatures(msg) =>
        dispatch(
          Actions.LanguageSupport(Feature_LanguageSupport.Msg.exthost(msg)),
        );
        Lwt.return(Reply.okEmpty);

      | MessageService(msg) =>
        Feature_Messages.Msg.exthost(
          ~dispatch=msg => dispatch(Actions.Messages(msg)),
          msg,
        )
        |> Lwt.map(
             fun
             | None => Reply.okEmpty
             | Some(handle) =>
               Reply.okJson(Exthost.Message.handleToJson(handle)),
           )

      | QuickOpen(msg) =>
        switch (msg) {
        | QuickOpen.Show({instance, _}) =>
          let (promise, resolver) = Lwt.task();
          dispatch(
            QuickmenuShow(
              Extension({id: instance, hasItems: false, resolver}),
            ),
          );

          promise |> Lwt.map(handle => Reply.okJson(`Int(handle)));
        | QuickOpen.SetItems({instance, items}) =>
          dispatch(QuickmenuUpdateExtensionItems({id: instance, items}));
          Lwt.return(Reply.okEmpty);
        | msg =>
          // TODO: Additional quick open messages
          Log.warnf(m =>
            m(
              "Unhandled QuickOpen message: %s",
              Exthost.Msg.QuickOpen.show_msg(msg),
            )
          );
          Lwt.return(Reply.okEmpty);
        }

      | StatusBar(
          SetEntry({
            id,
            label,
            alignment,
            priority,
            backgroundColor,
            color,
            command,
            tooltip,
            _,
          }),
        ) =>
        let command =
          command |> OptionEx.flatMap(({id, _}: Exthost.Command.t) => id);
        dispatch(
          Actions.StatusBar(
            Feature_StatusBar.Msg.itemAdded(
              Feature_StatusBar.Item.create(
                ~command?,
                ~color?,
                ~backgroundColor?,
                ~tooltip?,
                ~id,
                ~label,
                ~alignment,
                ~priority,
                (),
              ),
            ),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | StatusBar(Dispose({id})) =>
        dispatch(
          Actions.StatusBar(
            Feature_StatusBar.Msg.itemDisposed(id |> string_of_int),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | TerminalService(msg) =>
        Service_Terminal.handleExtensionMessage(msg);
        Lwt.return(Reply.okEmpty);
      | Window(OpenUri({uri})) =>
        Service_OS.Api.openURL(uri |> Oni_Core.Uri.toString)
          ? Lwt.return(Reply.okEmpty)
          : Lwt.return(Reply.error("Unable to open URI"))
      | Window(GetWindowVisibility) =>
        Lwt.return(Reply.okJson(`Bool(true)))

      | Workspace(SaveAll({includeUntitled})) =>
        ignore(includeUntitled);

        dispatch(
          Actions.VimExecuteCommand({command: "wa!", allowAnimation: false}),
        );
        Lwt.return(Reply.okEmpty);

      | Workspace(
          StartFileSearch({includePattern, excludePattern, maxResults}),
        ) =>
        Service_OS.Api.glob(
          ~maxCount=?maxResults,
          ~includeFiles=?includePattern,
          ~excludeDirectories=?excludePattern,
          // TODO: Pull from store
          Sys.getcwd(),
        )
        |> Lwt.map(paths =>
             Reply.okJson(
               `List(
                 paths
                 |> List.map(str =>
                      Oni_Core.Uri.fromPath(str) |> Oni_Core.Uri.to_yojson
                    ),
               ),
             )
           )
      | unhandledMsg =>
        Log.warnf(m =>
          m("Unhandled message: %s", Exthost.Msg.show(unhandledMsg))
        );
        Lwt.return(Reply.okEmpty);
      };
    };

  let parentPid = Luv.Pid.getpid();
  let name = Printf.sprintf("exthost-client-%s", parentPid |> string_of_int);
  let namedPipe = name |> NamedPipe.create;
  let pipeStr = NamedPipe.toString(namedPipe);

  let tempDir = Filename.get_temp_dir_name();

  let logFile = tempDir |> Uri.fromPath;
  let logsLocation =
    Filename.temp_file(~temp_dir=tempDir, "onivim2", "exthost.log")
    |> Uri.fromPath;

  let extHostVersion = {
    // The @onivim/vscode-exthost has an adjusted patch version number -
    // @onivim/vscode-exthost at 1.51.1000 corresponds to 1.51.1 of the vscode extension host
    let originalVersion = Oni_Core.BuildInfo.extensionHostVersion;

    originalVersion
    |> Semver.of_string
    |> OptionEx.flatMap((ver: Semver.t) => {
         Semver.from_parts(
           ver.major,
           ver.minor,
           ver.patch / 1000,
           ver.prerelease,
           ver.build,
         )
       })
    |> Option.map(Semver.to_string)
    |> OptionEx.tapNone(() =>
         Log.errorf(m => m("Unable to adjust version: %s", originalVersion))
       )
    |> Option.value(~default=originalVersion);
  };

  let staticWorkspace =
    initialWorkspace
    |> Option.map(({id, name, _}: Exthost.WorkspaceData.t) => {
         Exthost.Extension.InitData.StaticWorkspaceData.{id, name}
       })
    |> Option.value(
         ~default=Exthost.Extension.InitData.StaticWorkspaceData.global,
       );

  let initData =
    InitData.create(
      ~version=extHostVersion,
      ~parentPid,
      ~logsLocation,
      ~logFile,
      ~workspace=staticWorkspace,
      extensionInfo,
    );

  let onError = err => {
    Log.error(err);
  };

  let client =
    Exthost.Client.start(
      ~initialConfiguration=
        Feature_Configuration.toExtensionConfiguration(
          ~additionalExtensions=extensions,
          config,
        ),
      ~initialWorkspace,
      ~namedPipe,
      ~initData,
      ~handler,
      ~onError,
      (),
    );

  // INVESTIGATE: Why does using `Luv.Env.environ()` sometimes not work correctly when calling Process.spawn?
  // In some cases - intermittently - the spawned process will not have the environment variables set.
  // let env = Luv.Env.environ() |> Result.get_ok;
  // ...in the meantime, fall-back to Unix.environment:
  let env =
    Unix.environment()
    |> Array.to_list
    |> List.fold_left(
         (acc, curr) => {
           switch (String.split_on_char('=', curr)) {
           | [] => acc
           | [_] => acc
           | [key, ...values] =>
             let v = String.concat("=", values);

             [(key, v), ...acc];
           }
         },
         [],
       );
  let environment = [
    (
      "VSCODE_AMD_ENTRYPOINT",
      "vs/workbench/services/extensions/node/extensionHostProcess",
    ),
    ("VSCODE_IPC_HOOK_EXTHOST", pipeStr),
    ("VSCODE_PARENT_PID", parentPid |> string_of_int),
    ...env,
  ];

  let nodePath = Setup.(setup.nodePath);
  let extHostScriptPath = Setup.getNodeExtensionHostPath(setup);

  let on_exit = (_, ~exit_status: int64, ~term_signal) => {
    Log.infof(m =>
      m(
        "Extension host process exited with exit status: %Ld and signal: %d",
        exit_status,
        term_signal,
      )
    );
  };

  let redirect =
    if (attachStdio) {
      [
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stdin,
          ~from_parent_fd=Luv.Process.stdin,
          (),
        ),
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stdout,
          ~from_parent_fd=Luv.Process.stderr,
          (),
        ),
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stderr,
          ~from_parent_fd=Luv.Process.stderr,
          (),
        ),
      ];
    } else {
      [];
    };

  let _process: Luv.Process.t =
    LuvEx.Process.spawn(
      ~environment,
      ~on_exit,
      ~redirect,
      nodePath,
      [nodePath, extHostScriptPath],
    )
    // TODO: More robust error handling
    |> Result.get_ok;

  client |> Result.iter(c => maybeClientRef := Some(c));

  (client, stream);
};
