open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

type model = {
  fileExplorer: Component_FileExplorer.model,
  todo: unit,
};

[@deriving show]
type msg =
  | KeyboardInput(string)
  | FileExplorer(Component_FileExplorer.msg);

module Msg = {
  let keyPressed = key => KeyboardInput(key);
  let activeFileChanged = path =>
    FileExplorer(Component_FileExplorer.Msg.activeFileChanged(path));
};

let initial = (~rootPath) => {
  fileExplorer: Component_FileExplorer.initial(~rootPath),
  todo: (),
};

let setRoot = (~rootPath, {fileExplorer, _} as model) => {
  ...model,
  fileExplorer: Component_FileExplorer.setRoot(~rootPath, fileExplorer),
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | GrabFocus;

let update = (~configuration, msg, model) => {
  switch (msg) {
  | KeyboardInput(_) =>
    // Anything to be brought back here?
    (model, Nothing)

  | FileExplorer(fileExplorerMsg) =>
    let (fileExplorer, outmsg) =
      Component_FileExplorer.update(
        ~configuration,
        fileExplorerMsg,
        model.fileExplorer,
      );

    let outmsg' =
      switch (outmsg) {
      | Component_FileExplorer.Nothing => Nothing
      | Component_FileExplorer.Effect(eff) =>
        Effect(eff |> Isolinear.Effect.map(msg => FileExplorer(msg)))
      | Component_FileExplorer.OpenFile(path) => OpenFile(path)
      | GrabFocus => GrabFocus
      };

    // TODO: Wire up outmsg
    ({...model, fileExplorer}, outmsg');
  };
};

module View = {
  let make =
      (
        ~isFocused,
        ~iconTheme,
        ~languageInfo,
        ~model,
        ~decorations,
        ~theme,
        ~font,
        ~dispatch: msg => unit,
        (),
      ) => {
    <Component_FileExplorer.View
      isFocused
      iconTheme
      languageInfo
      decorations
      model={model.fileExplorer}
      theme
      font
      dispatch={msg => dispatch(FileExplorer(msg))}
    />;
  };
};

let sub = (~configuration, model) => {
  Component_FileExplorer.sub(~configuration, model.fileExplorer)
  |> Isolinear.Sub.map(msg => FileExplorer(msg));
};

module Contributions = {
  let commands = (~isFocused) => {
    !isFocused
      ? []
      : Component_FileExplorer.Contributions.commands(~isFocused)
        |> List.map(Oni_Core.Command.map(msg => FileExplorer(msg)));
  };

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;

    let fileExplorerKeys =
      isFocused
        ? Component_FileExplorer.Contributions.contextKeys(
            ~isFocused,
            model.fileExplorer,
          )
        : empty;

    fileExplorerKeys;
  };
};
