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
module Log = (val Log.withNamespace("Feature_Workspace"));

[@deriving show]
type command =
  | CloseFolder
  | OpenFolder;

[@deriving show]
type msg =
  | Command(command)
  | FolderSelectionCanceled
  | FolderPicked([@opaque] Fp.t(Fp.absolute))
  | WorkingDirectoryChanged(string);

module Msg = {
  let workingDirectoryChanged = workingDirectory =>
    WorkingDirectoryChanged(workingDirectory);
};

type model = {
  workingDirectory: string,
  openedFolder: option(string),
  rootName: string,
};

let initial = (~openedFolder: option(string), workingDirectory) => {
  workingDirectory,
  openedFolder,
  rootName: Filename.basename(workingDirectory),
};

let openedFolder = ({openedFolder, _}) => openedFolder;

let workingDirectory = ({workingDirectory, _}) => workingDirectory;

let rootName = ({rootName, _}) => rootName;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | WorkspaceChanged(option(string));

module Effects = {
  let changeDirectory = (path: Fp.t(Fp.absolute)) =>
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Workspace.changeDirectory", dispatch => {
      let newDirectory = Fp.toString(path);
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
      },
      WorkspaceChanged(Some(workingDirectory)),
    )
  | Command(CloseFolder) => (
      {...model, rootName: "", openedFolder: None},
      WorkspaceChanged(None),
    )
  | Command(OpenFolder) => (model, Effect(Effects.pickFolder))
  | FolderPicked(path) => (model, Effect(Effects.changeDirectory(path)))
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

  let all = model =>
    model.openedFolder == None
      ? [openFolder]
      // Always show open folder, so the user can switch folders from command palette
      : [openFolder, closeFolder];
};

module Contributions = {
  let commands = model => Commands.all(model);
};
