open Oni_Core;
open Oni_Core.Utility;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

include Model;

let getFileIcon = (~languageInfo, ~iconTheme, filePath) => {
  Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
  |> IconTheme.getIconForFile(iconTheme, filePath);
};

module Internal = {
  let sortByLoweredDisplayName = (a: FsTreeNode.t, b: FsTreeNode.t) => {
    switch (a.kind, b.kind) {
    | (Directory(_), File) => (-1)
    | (File, Directory(_)) => 1
    | _ =>
      compare(
        a.displayName |> String.lowercase_ascii,
        b.displayName |> String.lowercase_ascii,
      )
    };
  };

  let luvDirentToFsTree = (~getIcon, ~cwd, {name, kind}: Luv.File.Dirent.t) => {
    let path = Filename.concat(cwd, name);
    if (kind == `FILE || kind == `LINK) {
      Some(FsTreeNode.file(path, ~icon=getIcon(path)));
    } else if (kind == `DIR) {
      Some(FsTreeNode.directory(path, ~icon=getIcon(path), ~children=[]));
    } else {
      None;
    };
  };

  let luvDirentsToFsTree = (~getIcon, ~cwd, ~ignored, dirents) => {
    dirents
    |> List.filter(({name, _}: Luv.File.Dirent.t) =>
         name != ".." && name != "." && !List.mem(name, ignored)
       )
    |> List.filter_map(luvDirentToFsTree(~getIcon, ~cwd))
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
  let getFilesAndFolders = (~ignored, cwd, getIcon) => {
    cwd
    |> Service_OS.Api.readdir
    |> Lwt.map(luvDirentsToFsTree(~ignored, ~cwd, ~getIcon));
  };

  let getDirectoryTree = (cwd, languageInfo, iconTheme, ignored) => {
    let getIcon = getFileIcon(~languageInfo, ~iconTheme);
    let childrenPromise = getFilesAndFolders(~ignored, cwd, getIcon);

    childrenPromise
    |> Lwt.map(children => {
         FsTreeNode.directory(
           cwd,
           ~icon=getIcon(cwd),
           ~children,
           ~isOpen=true,
         )
       });
  };
};

module Effects = {
  let load = (directory, languageInfo, iconTheme, configuration, ~onComplete) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.load", dispatch => {
      let ignored =
        Configuration.getValue(c => c.filesExclude, configuration);
      let promise =
        Internal.getDirectoryTree(
          directory,
          languageInfo,
          iconTheme,
          ignored,
        );

      Lwt.on_success(promise, tree => {dispatch(onComplete(tree))});
    });
  };
};

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | GrabFocus;

let setTree = (tree, model) => {...model, tree: Some(tree)};

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
let setScrollOffset = (scrollOffset, model) => {...model, scrollOffset};

let replaceNode = (node, model: model) =>
  switch (model.tree) {
  | Some(tree) =>
    setTree(FsTreeNode.replace(~replacement=node, tree), model)
  | None => model
  };

let revealAndFocusPath =
    (~languageInfo, ~iconTheme, ~configuration, path, model: model) => {
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
            lastNode.path,
            languageInfo,
            iconTheme,
            configuration,
            ~onComplete=node =>
            FocusNodeLoaded(node)
          ),
        ),
      )

    // Open ALL the nodes (in the path)!
    | `Success(_) =>
      let tree = FsTreeNode.updateNodesInPath(FsTreeNode.setOpen, path, tree);
      let offset =
        switch (FsTreeNode.expandedIndex(path, tree)) {
        | Some(offset) => `Middle(float(offset))
        | None => model.scrollOffset
        };

      (
        model
        |> setFocus(Some(path))
        |> setTree(tree)
        |> setScrollOffset(offset),
        Nothing,
      );
    }

  | None => (model, Nothing)
  };
};

let revealFocus = model => {
  switch (model.focus, model.tree) {
  | (Some(focus), Some(tree)) =>
    switch (FsTreeNode.expandedIndex(focus, tree)) {
    | Some(index) => {...model, scrollOffset: `Reveal(index)}
    | None => model
    }
  | _ => model
  };
};

let selectNode =
    (~languageInfo, ~iconTheme, ~configuration, node: FsTreeNode.t, model) =>
  switch (node) {
  | {kind: File, path, _} =>
    // Set active here to avoid scrolling in BufferEnter
    (model |> setActive(Some(node.path)), OpenFile(path))

  | {kind: Directory({isOpen, _}), _} => (
      replaceNode(FsTreeNode.toggleOpen(node), model),
      isOpen
        ? Nothing
        : Effect(
            Effects.load(
              node.path,
              languageInfo,
              iconTheme,
              configuration,
              ~onComplete=newNode =>
              NodeLoaded(newNode)
            ),
          ),
    )
  };

let update = (~configuration, ~languageInfo, ~iconTheme, msg, model) => {
  switch (msg) {
  | ActiveFilePathChanged(maybeFilePath) =>
    switch (model) {
    | {active, _} when active != maybeFilePath =>
      switch (maybeFilePath) {
      | Some(path) =>
        let autoReveal =
          Oni_Core.Configuration.getValue(
            c => c.explorerAutoReveal,
            configuration,
          );
        switch (autoReveal) {
        | `HighlightAndScroll =>
          let model' = {...model, active: Some(path)};
          revealAndFocusPath(
            ~languageInfo,
            ~configuration,
            ~iconTheme,
            path,
            model',
          );
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
      |> revealAndFocusPath(
           ~languageInfo,
           ~configuration,
           ~iconTheme,
           activePath,
         )

    | None => (model, Nothing)
    }

  | NodeClicked(node) =>
    model
    |> setFocus(Some(node.path))
    |> selectNode(~languageInfo, ~configuration, ~iconTheme, node)

  | ScrollOffsetChanged(offset) => (setScrollOffset(offset, model), Nothing)

  | KeyboardInput(key) =>
    let handleKey = ((path, tree)) =>
      switch (key) {
      | "<CR>" =>
        FsTreeNode.findByPath(path, tree)
        |> Option.map(node =>
             selectNode(
               ~languageInfo,
               ~configuration,
               ~iconTheme,
               node,
               model,
             )
           )

      | "<UP>" =>
        FsTreeNode.prevExpandedNode(path, tree)
        |> Option.map((node: FsTreeNode.t) =>
             (model |> setFocus(Some(node.path)) |> revealFocus, Nothing)
           )

      | "<DOWN>" =>
        FsTreeNode.nextExpandedNode(path, tree)
        |> Option.map((node: FsTreeNode.t) =>
             (model |> setFocus(Some(node.path)) |> revealFocus, Nothing)
           )

      | _ => None
      };

    OptionEx.zip(model.focus, model.tree)
    |> OptionEx.flatMap(handleKey)
    |> Option.value(~default=(model, Nothing));
  };
};

module View = View;

let sub = (~configuration, ~languageInfo, ~iconTheme, {rootPath, _}) => {
  let getIcon = getFileIcon(~languageInfo, ~iconTheme);
  let ignored = Configuration.getValue(c => c.filesExclude, configuration);

  let toMsg =
    fun
    | Ok(dirents) => {
        let children =
          dirents
          |> Internal.luvDirentsToFsTree(~ignored, ~getIcon, ~cwd=rootPath);
        TreeLoaded(
          FsTreeNode.directory(
            rootPath,
            ~icon=getIcon(rootPath),
            ~children,
            ~isOpen=true,
          ),
        );
      }
    | Error(msg) => TreeLoadError(msg);

  Service_OS.Sub.dir(~uniqueId="FileExplorerSideBar", ~toMsg, rootPath);
};
