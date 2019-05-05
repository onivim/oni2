/**
   FileExplorerStoreConnector.re

   Implements an updater (reducer + side effects) for the Menu
 */
open Oni_Core;
open Oni_Model;

let checkIsDirectory = dir =>
  try (Sys.is_directory(dir)) {
  | Sys_error(error) =>
    print_endline(error);
    false;
  };

let rec getFiles = (cwd, getIcon, ~ignored) => {
  try%lwt (
    Lwt_unix.files_of_directory(cwd)
    /* Filter out the relative name for current and parent directory*/
    |> Lwt_stream.filter(name => name != ".." && name != ".")
    /* Remove ignored files from search */
    |> Lwt_stream.filter(name => !List.mem(name, ignored))
    |> Lwt_stream.to_list
    |> (
      promise =>
        Lwt.bind(promise, files =>
          Lwt_list.map_p(
            file => {
              let path = Filename.concat(cwd, file);
              let isDirectory = checkIsDirectory(path);

              let%lwt children =
                isDirectory
                  ? getFiles(~ignored, path, getIcon) : Lwt.return([]);

              FileExplorer.createFsNode(
                ~path,
                ~displayName=file,
                ~children,
                ~isDirectory,
                ~fileIcon=getIcon(path),
              )
              |> Lwt.return;
            },
            files,
          )
        )
    )
  )
    {
    | Failure(e) =>
      print_endline(e);
      Lwt.return([]);
    };
    /* | Unix.Unix_error(_, _, message) => */
    /*   print_endline("Error: " ++ message); */
    /*   Lwt.return([]); */
};

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let getExplorerFilesEffect = (cwd, languageInfo, iconTheme, ignored) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.open", dispatch => {
      let getIcon = FileExplorer.getFileIcon(languageInfo, iconTheme);
      let ignored = ["node_modules", "_esy"];
      let directory = getFiles(~ignored, cwd, getIcon) |> Lwt_main.run;
      let newTree =
        FileExplorer.createFsNode(
          ~path=cwd,
          ~displayName=Filename.basename(cwd),
          ~isDirectory=true,
          ~children=directory,
          ~fileIcon=getIcon(cwd),
        )
        |> FileExplorer.listToTree(directory);
      dispatch(Actions.SetExplorerTree(newTree));
    });
  };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | OpenExplorer(directory) => (
        state,
        getExplorerFilesEffect(
          directory,
          state.languageInfo,
          state.iconTheme,
          ["node_modules", "_esy"],
        ),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };
  (updater, stream);
};
