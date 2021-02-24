/*
 * A workspace models the current 'ambient environment' of the editor, in particular:
 * - Current directory
 *
 * ...and eventually
 * - Open editors
 * - Local modifications (hot exit)
 * - Per-workspace configuration
 */

open Oni_Core;
open Utility;
module Log = (val Log.withNamespace("Feature_Workspace"));

[@deriving show]
type command =
  | CloseFolder
  | OpenFolder;

[@deriving show]
type msg =
  | Command(command)
  | FolderSelectionCanceled
  | FolderPicked([@opaque] FpExp.t(FpExp.absolute))
  | WorkingDirectoryChanged(string)
  | Noop;

module Msg = {
  let workingDirectoryChanged = workingDirectory =>
    WorkingDirectoryChanged(workingDirectory);
};

type model = {
  workingDirectory: string,
  openedFolder: option(string),
  rootName: string,
  lastPickFromFilePicker: bool,
};

let initial = (~openedFolder: option(string), workingDirectory) => {
  workingDirectory,
  openedFolder,
  rootName: Filename.basename(workingDirectory),
  lastPickFromFilePicker: false,
};

let openedFolder = ({openedFolder, _}) => openedFolder;

let workingDirectory = ({workingDirectory, _}) => workingDirectory;

let rootName = ({rootName, _}) => rootName;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | WorkspaceChanged({
      path: option(string),
      shouldFocusExplorer: bool,
    });

module Effects = {
  let changeDirectory = (path: FpExp.t(FpExp.absolute)) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Workspace.changeDirectory", dispatch => {
      let newDirectory = FpExp.toString(path);
      switch (Luv.Path.chdir(newDirectory)) {
      | Ok () => dispatch(WorkingDirectoryChanged(newDirectory))
      | Error(msg) =>
        Log.errorf(m =>
          m("Error changing directory: %s\n", Luv.Error.strerror(msg))
        )
      };
    });

  let pickFolder =
    Service_OS.Effect.Dialog.openFolder(
      fun
      | None => FolderSelectionCanceled
      | Some(fp) => FolderPicked(fp),
    );
};

let update = (msg, model) => {
  switch (msg) {
  | FolderSelectionCanceled => (model, Nothing)

  | WorkingDirectoryChanged(workingDirectory) => (
      {
        workingDirectory,
        rootName: Filename.basename(workingDirectory),
        openedFolder: Some(workingDirectory),
        // Reset lastPickFromFilePicker once we've actually changed
        lastPickFromFilePicker: false,
      },
      WorkspaceChanged({
        path: Some(workingDirectory),
        // If the workspace change came from the open-folder dialog,
        // focus the explorer.
        shouldFocusExplorer: model.lastPickFromFilePicker,
      }),
    )

  | Command(CloseFolder) => (
      {...model, rootName: "", openedFolder: None},
      WorkspaceChanged({path: None, shouldFocusExplorer: false}),
    )

  | Command(OpenFolder) => (model, Effect(Effects.pickFolder))

  | FolderPicked(path) => (
      {...model, lastPickFromFilePicker: true},
      Effect(Effects.changeDirectory(path)),
    )

  | Noop => (model, Nothing)
  };
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let closeFolder =
    define(
      ~category="Workspace",
      ~title="Close Folder",
      "workspace.action.closeFolder",
      Command(CloseFolder),
    );

  let openFolder =
    define(
      ~category="Workspace",
      ~title="Open Folder",
      "_workbench.pickWorkspaceFolder",
      Command(OpenFolder),
    );

  let openFolderArgs =
    defineWithArgs(
      "vscode.openFolder",
      fun
      | `List([uriJson, ..._]) =>
        uriJson
        |> Json.Decode.decode_value(Uri.decode)
        |> Result.map(Uri.toFileSystemPath)
        |> Result.to_option
        |> OptionEx.flatMap(FpExp.absoluteCurrentPlatform)
        |> Option.map(fp => FolderPicked(fp))
        |> Option.value(~default=Noop)
      | _ => Noop,
    );

  let all = model =>
    model.openedFolder == None
      ? [openFolder, openFolderArgs]
      // Always show open folder, so the user can switch folders from command palette
      : [openFolder, openFolderArgs, closeFolder];
};

module MenuItems = {
  open MenuBar.Schema;
  open Feature_MenuBar;

  let openFolder = command(Commands.openFolder);
  let closeFolder = command(Commands.closeFolder);

  let group = group(~parent=Global.file, [openFolder, closeFolder]);
};

module Contributions = {
  let commands = model => Commands.all(model);

  let menuGroup = MenuItems.group;
};
