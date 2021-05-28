open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

include Model;

module Configuration = {
  open Config.Schema;

  let useFileWatcher =
    setting("files.useExperimentalFileWatcher", bool, ~default=true);
};

module Internal = {
  let sortByLoweredDisplayName = (a: FsTreeNode.t, b: FsTreeNode.t) => {
    switch (a, b) {
    | (Node(_), Leaf(_)) => (-1)
    | (Leaf(_), Node(_)) => 1
    | (Node({data: a, _}), Node({data: b, _}))
    | (Leaf(a), Leaf(b)) =>
      compare(
        a.displayName |> String.lowercase_ascii,
        b.displayName |> String.lowercase_ascii,
      )
    };
  };

  let luvDirentToFsTree = (dirent: Service_OS.DirectoryEntry.t) => {
    open Service_OS;

    let path = DirectoryEntry.path(dirent);
    let isSymlink = DirectoryEntry.isSymbolicLink(dirent);
    if (DirectoryEntry.isFile(dirent)) {
      Some(FsTreeNode.file(~isSymlink, path));
    } else if (DirectoryEntry.isDirectory(dirent)) {
      Some(FsTreeNode.directory(~isSymlink, path, ~children=[]));
    } else {
      None;
    };
  };

  let luvDirentsToFsTree = (~ignored, dirents) => {
    Service_OS.(
      dirents
      |> List.filter((dirEnt: DirectoryEntry.t) => {
           let name = DirectoryEntry.name(dirEnt);
           name != ".." && name != "." && !List.mem(name, ignored);
         })
      |> List.filter_map(luvDirentToFsTree)
      |> List.sort(sortByLoweredDisplayName)
    );
  };

  /**
    getFilesAndFolders

     This function uses Lwt to get all the files and folders in a directory
     then for each we check if it is a file or folder.
     if it is a directory we recursively call getFilesAndFolders on it
     to resolves its subfolders and files. We do this concurrently using
     Lwt_list.map_p. The recursion is gated by the depth value so it does
     not recurse too far.
   */
  let getFilesAndFolders = (~ignored, cwd) => {
    cwd |> Service_OS.Api.readdir2 |> Lwt.map(luvDirentsToFsTree(~ignored));
  };

  let getDirectoryTree = (cwd: FpExp.t(FpExp.absolute), ignored) => {
    let childrenPromise = getFilesAndFolders(~ignored, cwd);

    childrenPromise
    |> Lwt.map(children => {
         FsTreeNode.directory(
           // HACK: We don't know if this is a symlink,
           // and the merge logic assumes that the symlink status
           // doesn't change. So we skip an extra stat to check the symlink
           // status.
           ~isSymlink=false,
           cwd,
           ~children,
           ~isOpen=true,
         )
       });
  };
};

module Effects = {
  let load = (directory, config, ~onComplete) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.load", dispatch => {
      let directoryStr = FpExp.toString(directory);
      Log.infof(m => m("Loading nodes for directory: %s", directoryStr));
      let ignored =
        Feature_Configuration.GlobalConfiguration.Files.exclude.get(config);
      let promise = Internal.getDirectoryTree(directory, ignored);

      Lwt.on_success(
        promise,
        tree => {
          Log.infof(m =>
            m("Successfully loaded nodes for directory: %s", directoryStr)
          );
          dispatch(onComplete(tree));
        },
      );

      Lwt.on_failure(promise, exn => {
        Log.errorf(m =>
          m(
            "Error loading directory %s: %s",
            directoryStr,
            Printexc.to_string(exn),
          )
        )
      });
    });
  };
};

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | PreviewFile(string)
  | GrabFocus
  | WatchedPathChanged({
      path: FpExp.t(FpExp.absolute),
      stat: option(Luv.File.Stat.t),
    });

let setTree = (tree, model) => {
  let uniqueId = (data: FsTreeNode.metadata) => FpExp.toString(data.path);
  let (rootName, firstLevelChildren) =
    switch (tree) {
    | Tree.Leaf(_) => ("", [])
    | Tree.Node({children, data, _}) => (
        FsTreeNode.(data.displayName),
        children,
      )
    };
  let searchText =
      (
        node:
          Component_VimTree.nodeOrLeaf(
            FsTreeNode.metadata,
            FsTreeNode.metadata,
          ),
      ) => {
    switch (node) {
    | Leaf({data, _}) => data.displayName
    | Node({data, _}) => data.displayName
    };
  };
  let treeView =
    Component_VimTree.set(
      ~searchText,
      ~uniqueId,
      firstLevelChildren,
      model.treeView,
    );

  {...model, rootName, tree: Some(tree), treeView};
};

let keyPress = (key, model) => {
  ...model,
  treeView: Component_VimTree.keyPress(key, model.treeView),
};

let scrollTo = (~index, ~alignment, model) => {
  {
    ...model,
    treeView: Component_VimTree.scrollTo(~index, ~alignment, model.treeView),
  };
};

let setActive = (maybePath, model) => {...model, active: maybePath};

let setFocus = (maybePath, model) =>
  switch (maybePath, model.tree) {
  | (Some(path), Some(tree)) =>
    switch (FsTreeNode.findByPath(path, tree)) {
    | Some(_) => {...model, focus: Some(path)}
    | None => model
    }
  | _ => {...model, focus: None}
  };

let replaceNode = (node, model: model) =>
  switch (model.tree) {
  | Some(tree) =>
    setTree(FsTreeNode.replace(~replacement=node, tree), model)
  | None => setTree(node, model)
  };

let revealAndFocusPath = (~config, path, model: model) => {
  switch (model.tree) {
  | Some(tree) =>
    switch (FsTreeNode.findNodesByPath(path, tree)) {
    // Nothing to do
    | `Success([])
    | `Failed => (model, Nothing)

    // Load next unloaded node in path
    | `Partial(lastNode) => (
        model,
        Effect(
          Effects.load(FsTreeNode.getPath(lastNode), config, ~onComplete=node =>
            FocusNodeLoaded(node)
          ),
        ),
      )

    // Open ALL the nodes (in the path)!
    | `Success(_) =>
      let tree = FsTreeNode.updateNodesInPath(FsTreeNode.setOpen, path, tree);

      let model = model |> setFocus(Some(path)) |> setTree(tree);
      let maybePathIndex = getIndex(path, model);

      let scrolledModel =
        maybePathIndex
        |> Option.map(index => scrollTo(~index, ~alignment=`Center, model))
        |> Option.value(~default=model);

      (scrolledModel, Nothing);
    }

  | None => (model, Nothing)
  };
};

let expand = (path, model) => {
  let expandedPaths =
    [path, ...model.expandedPaths]
    |> Base.List.dedup_and_sort(~compare=FpExp.compare);

  let pathsToLoad =
    [path, ...model.pathsToLoad]
    |> Base.List.dedup_and_sort(~compare=FpExp.compare);
  {...model, expandedPaths, pathsToLoad};
};

let collapse = (path, model) => {
  let expandedPaths = model.expandedPaths |> List.filter(p => p != path);
  {...model, expandedPaths};
};

let markNodeAsLoaded = (node, model) => {
  let pathsToLoad =
    model.pathsToLoad
    |> List.filter(p => !FpExp.eq(FsTreeNode.getPath(node), p));
  {...model, pathsToLoad};
};

let reload = model => {...model, pathsToLoad: model.expandedPaths};

let update = (~config, msg, model) => {
  switch (msg) {
  | FileWatcherEvent({path, event}) => (
      model |> expand(path),
      WatchedPathChanged({path, stat: event.stat}),
    )
  | ActiveFilePathChanged(maybeFilePath) =>
    switch (model) {
    | {active, _} when active != maybeFilePath =>
      switch (maybeFilePath) {
      | Some(path) =>
        let autoReveal =
          Feature_Configuration.GlobalConfiguration.Explorer.autoReveal.get(
            config,
          );
        switch (autoReveal) {
        | `HighlightAndScroll =>
          let model' = {...model, active: Some(path)};
          revealAndFocusPath(~config, path, model');
        | `HighlightOnly =>
          let model = setActive(Some(path), model);
          (setFocus(Some(path), model), Nothing);
        | `NoReveal => (model, Nothing)
        };
      | None => (model, Nothing)
      }
    | _ => (model, Nothing)
    }

  | NodeLoadError(_msg) => (model, Nothing)

  | NodeLoaded(node) => (
      model |> replaceNode(node) |> markNodeAsLoaded(node),
      Nothing,
    )

  | FocusNodeLoaded(node) =>
    switch (model.active) {
    | Some(activePath) =>
      model
      |> expand(FsTreeNode.getPath(node))
      |> replaceNode(node)
      |> markNodeAsLoaded(node)
      |> revealAndFocusPath(~config, activePath)

    | None => (model, Nothing)
    }

  | Command(Reload) => (reload(model), Nothing)

  | Tree(treeMsg) =>
    let (treeView, outmsg) =
      Component_VimTree.update(treeMsg, model.treeView);

    let model = {...model, treeView};
    switch (outmsg) {
    | Component_VimTree.Expanded(node) => (
        model |> expand(node.path),
        Nothing,
      )
    | Component_VimTree.Collapsed(node) => (
        model |> collapse(node.path),
        Nothing,
      )
    | Component_VimTree.Touched(node) =>
      // Set active here to avoid scrolling in BufferEnter
      (
        model |> setActive(Some(node.path)),
        Feature_Configuration.GlobalConfiguration.Workbench.editorEnablePreview.
          get(
          config,
        )
          ? PreviewFile(FpExp.toString(node.path))
          : OpenFile(FpExp.toString(node.path)),
      )
    | Component_VimTree.Selected(node) =>
      // Set active here to avoid scrolling in BufferEnter
      (
        model |> setActive(Some(node.path)),
        OpenFile(FpExp.toString(node.path)),
      )
    | Component_VimTree.SelectedNode(_) => (model, Nothing)
    | Component_VimTree.Nothing => (model, Nothing)
    };
  };
};

module View = View;

let sub = (~config, {fileWatcherKey, expandedPaths, pathsToLoad, _}) => {
  let ignored =
    Feature_Configuration.GlobalConfiguration.Files.exclude.get(config);

  let toMsg = path =>
    fun
    | Ok(dirents) => {
        let children = dirents |> Internal.luvDirentsToFsTree(~ignored);
        NodeLoaded(
          FsTreeNode.directory(
            // HACK: We don't know if this is a symlink,
            // and the merge logic assumes that the symlink status
            // doesn't change. So we skip an extra stat to check the symlink
            // status.
            ~isSymlink=false,
            path,
            ~children,
            ~isOpen=true,
          ),
        );
      }
    | Error(msg) => NodeLoadError(msg);

  let allPathsToLoad = pathsToLoad;

  // Sort expanded paths so we load the root first, then subdirs, etc.
  let expandedPathSubs =
    allPathsToLoad
    |> List.sort((a, b) => {
         String.length(FpExp.toString(a))
         - String.length(FpExp.toString(b))
       })
    |> (
      l =>
        List.nth_opt(l, 0)
        |> Option.map(path => {
             Service_OS.Sub.dir(
               ~uniqueId="FileExplorer:Sub" ++ FpExp.toString(path),
               ~toMsg=toMsg(path),
               path,
             )
           })
        |> Option.value(~default=Isolinear.Sub.none)
    );

  let onEvent = (path, evt: Service_FileWatcher.event) => {
    FileWatcherEvent({path, event: evt});
  };

  let allPathsToWatch = expandedPaths;

  let useFileWatcher = Configuration.useFileWatcher.get(config);
  let watchers =
    !useFileWatcher
      ? [Isolinear.Sub.none]
      : allPathsToWatch
        |> List.map(path => {
             Service_FileWatcher.watch(
               ~watchChanges=false,
               ~key=fileWatcherKey,
               ~path,
               ~onEvent=onEvent(path),
             )
           });

  [expandedPathSubs, ...watchers] |> Isolinear.Sub.batch;
};

module Commands = {
  open Feature_Commands.Schema;

  let reload =
    define(
      ~category="Explorer",
      ~title="Reload",
      "workbench.todo.explorer-reload",
      Command(Reload),
    );
};

module Contributions = {
  let commands = (~isFocused) => {
    !isFocused
      ? []
      : [Commands.reload]
        @ (
          Component_VimTree.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => Tree(msg)))
        );
  };

  let configuration = Configuration.[useFileWatcher.spec];

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;

    let vimTreeKeys =
      isFocused
        ? Component_VimTree.Contributions.contextKeys(model.treeView) : empty;

    vimTreeKeys;
  };
};
