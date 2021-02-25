open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

include Model;

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

  let luvDirentToFsTree = (~cwd, {name, kind}: Luv.File.Dirent.t) => {
    let path = FpExp.At.(cwd / name);

    if (kind == `FILE || kind == `LINK) {
      Some(FsTreeNode.file(path));
    } else if (kind == `DIR) {
      Some(FsTreeNode.directory(path, ~children=[]));
    } else {
      None;
    };
  };

  let luvDirentsToFsTree = (~cwd: FpExp.t(FpExp.absolute), ~ignored, dirents) => {
    dirents
    |> List.filter(({name, _}: Luv.File.Dirent.t) =>
         name != ".." && name != "." && !List.mem(name, ignored)
       )
    |> List.filter_map(luvDirentToFsTree(~cwd))
    |> List.sort(sortByLoweredDisplayName);
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
    cwd
    |> FpExp.toString
    |> Service_OS.Api.readdir
    |> Lwt.map(luvDirentsToFsTree(~ignored, ~cwd));
  };

  let getDirectoryTree = (cwd: FpExp.t(FpExp.absolute), ignored) => {
    let childrenPromise = getFilesAndFolders(~ignored, cwd);

    childrenPromise
    |> Lwt.map(children => {
         FsTreeNode.directory(cwd, ~children, ~isOpen=true)
       });
  };
};

module Effects = {
  let load = (directory, configuration, ~onComplete) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.load", dispatch => {
      let directoryStr = FpExp.toString(directory);
      Log.infof(m => m("Loading nodes for directory: %s", directoryStr));
      let ignored =
        Feature_Configuration.Legacy.getValue(
          c => c.filesExclude,
          configuration,
        );
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
  | GrabFocus;

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
  | None => model
  };

let revealAndFocusPath = (~configuration, path, model: model) => {
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
          Effects.load(
            FsTreeNode.getPath(lastNode), configuration, ~onComplete=node =>
            FocusNodeLoaded(node)
          ),
        ),
      )

    // Open ALL the nodes (in the path)!
    | `Success(_) =>
      let tree = FsTreeNode.updateNodesInPath(FsTreeNode.setOpen, path, tree);

      let maybePathIndex = getIndex(path, model);

      let model = model |> setFocus(Some(path)) |> setTree(tree);

      let scrolledModel =
        maybePathIndex
        |> Option.map(index => scrollTo(~index, ~alignment=`Center, model))
        |> Option.value(~default=model);

      (scrolledModel, Nothing);
    }

  | None => (model, Nothing)
  };
};

let update = (~config, ~configuration, msg, model) => {
  switch (msg) {
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
          revealAndFocusPath(~configuration, path, model');
        | `HighlightOnly =>
          let model = setActive(Some(path), model);
          (setFocus(Some(path), model), Nothing);
        | `NoReveal => (model, Nothing)
        };
      | None => (model, Nothing)
      }
    | _ => (model, Nothing)
    }

  | TreeLoaded(tree) => (setTree(tree, model), Nothing)

  | TreeLoadError(_msg) => (model, Nothing)

  | NodeLoaded(node) => (replaceNode(node, model), Nothing)

  | FocusNodeLoaded(node) =>
    switch (model.active) {
    | Some(activePath) =>
      model
      |> replaceNode(node)
      |> revealAndFocusPath(~configuration, activePath)

    | None => (model, Nothing)
    }

  | KeyboardInput(_) =>
    // Anything to be brought back here?
    (model, Nothing)

  | Tree(treeMsg) =>
    let (treeView, outmsg) =
      Component_VimTree.update(treeMsg, model.treeView);

    let model = {...model, treeView};
    switch (outmsg) {
    | Component_VimTree.Expanded(node) => (
        model,
        Effect(
          Effects.load(node.path, configuration, ~onComplete=newNode =>
            NodeLoaded(newNode)
          ),
        ),
      )
    | Component_VimTree.Collapsed(_) => (model, Nothing)
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
    | Component_VimTree.Nothing => (model, Nothing)
    };
  };
};

module View = View;

let sub = (~configuration, {rootPath, _}) => {
  let ignored =
    Feature_Configuration.Legacy.getValue(c => c.filesExclude, configuration);

  let toMsg =
    fun
    | Ok(dirents) => {
        let children =
          dirents |> Internal.luvDirentsToFsTree(~ignored, ~cwd=rootPath);
        TreeLoaded(FsTreeNode.directory(rootPath, ~children, ~isOpen=true));
      }
    | Error(msg) => TreeLoadError(msg);

  Service_OS.Sub.dir(
    ~uniqueId="FileExplorerSideBar",
    ~toMsg,
    FpExp.toString(rootPath),
  );
};

module Contributions = {
  let commands = (~isFocused) => {
    !isFocused
      ? []
      : Component_VimTree.Contributions.commands
        |> List.map(Oni_Core.Command.map(msg => Tree(msg)));
  };

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;

    let vimTreeKeys =
      isFocused
        ? Component_VimTree.Contributions.contextKeys(model.treeView) : empty;

    vimTreeKeys;
  };
};
