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

type msg = 
[@deriving show]
| ChangeDirectory(string)
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

let update = (msg, model) => {
  switch (msg) {
  | WorkingDirectoryChanged(workingDirectory) => {
    workingDirectory,
    rootName: Filename.basename(workingDirectory),
    openedFolder: Some(workingDirectory),
  }
  }
}

module Effects = {
  let changeDirectory = (path: Fp.t(Fp.absolute)) => Isolinear.Effect.createWithDispatch(
    ~name="Feature_Workspace.changeDirectory",
    dispatch => {
      let newDirectory = Fp.toString(path);
      switch (Luv.Path.chdir(newDirectory)) {
      | Ok () => dispatch(WorkingDirectoryChanged(newDirectory))
      | Error(msg) =>
      Log.errorf(m => m("Error changing directory: %s\n", Luv.Error.strerror(msg)))
      }
      try({
        Sys.chdir(newDirectory);
        let outdir = Sys.getcwd();
        dispatch(WorkingDirectoryChanged(outdir));
      }) {
      | Sys_error(msg) => 
      };
    }
  )
}
