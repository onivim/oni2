open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

type model = {
  fileExplorer: Component_FileExplorer.model,
  symbolOutline:
    Component_VimTree.model(
      Feature_LanguageSupport.DocumentSymbols.symbol,
      Feature_LanguageSupport.DocumentSymbols.symbol,
    ),
};

[@deriving show]
type msg =
  | KeyboardInput(string)
  | FileExplorer(Component_FileExplorer.msg)
  | SymbolOutline(Component_VimTree.msg)
  | SymbolsChanged(
      [@opaque] option(Feature_LanguageSupport.DocumentSymbols.t),
    );

module Msg = {
  let keyPressed = key => KeyboardInput(key);
  let activeFileChanged = path =>
    FileExplorer(Component_FileExplorer.Msg.activeFileChanged(path));
};

let initial = (~rootPath) => {
  fileExplorer: Component_FileExplorer.initial(~rootPath),
  symbolOutline: Component_VimTree.create(~rowHeight=20),
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

    ({...model, fileExplorer}, outmsg');

  | SymbolsChanged(maybeSymbols) =>
    let symbols = maybeSymbols |> Option.value(~default=[]);

    // TODO
    let uniqueId = _ => "";
    (
      {
        ...model,
        symbolOutline:
          Component_VimTree.set(~uniqueId, symbols, model.symbolOutline),
      },
      Nothing,
    );

  | SymbolOutline(symbolMsg) =>
    let (symbolOutline, _outmsg) =
      Component_VimTree.update(symbolMsg, model.symbolOutline);

    ({...model, symbolOutline}, Nothing);
  };
};

module View = {
  open Revery.UI;
  open Revery.UI.Components;
  let%component make =
                (
                  ~isFocused,
                  ~iconTheme,
                  ~languageInfo,
                  ~model,
                  ~decorations,
                  ~documentSymbols:
                     option(Feature_LanguageSupport.DocumentSymbols.t),
                  ~theme,
                  ~font,
                  ~dispatch: msg => unit,
                  (),
                ) => {
    let%hook () =
      Hooks.effect(
        OnMountAndIf((!=), documentSymbols),
        () => {
          dispatch(SymbolsChanged(documentSymbols));
          None;
        },
      );

    let renderSymbol =
        (
          ~availableWidth as _,
          ~index as _,
          ~hovered as _,
          ~selected as _,
          nodeOrLeaf:
            Component_VimTree.nodeOrLeaf(
              Feature_LanguageSupport.DocumentSymbols.symbol,
              Feature_LanguageSupport.DocumentSymbols.symbol,
            ),
        ) => {
      let symbolData =
        switch (nodeOrLeaf) {
        | Component_VimTree.Leaf({data, _})
        | Component_VimTree.Node({data, _}) => data
        };

      <Oni_Components.Tooltip text={symbolData.detail}>
        <View
          style=Style.[
            flexDirection(`Row),
            justifyContent(`Center),
            alignItems(`Center),
          ]>
          <View style=Style.[padding(4)]>
            <Oni_Components.SymbolIcon theme symbol={symbolData.kind} />
          </View>
          <Text text={symbolData.name} />
        </View>
      </Oni_Components.Tooltip>;
    };

    <View style=Style.[flexDirection(`Column), flexGrow(1)]>
      <View style=Style.[flexGrow(2)]>
        <Component_FileExplorer.View
          isFocused
          iconTheme
          languageInfo
          decorations
          model={model.fileExplorer}
          theme
          font
          dispatch={msg => dispatch(FileExplorer(msg))}
        />
      </View>
      <Component_Accordion.VimTree
        title="Outline"
        expanded=true
        isFocused
        uiFont=font
        theme
        model={model.symbolOutline}
        render=renderSymbol
        onClick={() => ()}
        dispatch={msg => dispatch(SymbolOutline(msg))}
      />
    </View>;
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
