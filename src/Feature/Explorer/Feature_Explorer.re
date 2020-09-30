open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

type focus =
  | FileExplorer
  | Outline;

type model = {
  focus,
  fileExplorer: Component_FileExplorer.model,
  symbolOutline:
    Component_VimTree.model(
      Feature_LanguageSupport.DocumentSymbols.symbol,
      Feature_LanguageSupport.DocumentSymbols.symbol,
    ),
  vimWindowNavigation: Component_VimWindows.model,
};

[@deriving show]
type msg =
  | KeyboardInput(string)
  | FileExplorer(Component_FileExplorer.msg)
  | SymbolOutline(Component_VimTree.msg)
  | SymbolsChanged(
      [@opaque] option(Feature_LanguageSupport.DocumentSymbols.t),
    )
  | VimWindowNav(Component_VimWindows.msg);

module Msg = {
  let keyPressed = key => KeyboardInput(key);
  let activeFileChanged = path =>
    FileExplorer(Component_FileExplorer.Msg.activeFileChanged(path));
};

let initial = (~rootPath) => {
  focus: FileExplorer,
  fileExplorer: Component_FileExplorer.initial(~rootPath),
  symbolOutline: Component_VimTree.create(~rowHeight=20),
  vimWindowNavigation: Component_VimWindows.initial,
};

let setRoot = (~rootPath, {fileExplorer, _} as model) => {
  ...model,
  fileExplorer: Component_FileExplorer.setRoot(~rootPath, fileExplorer),
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | GrabFocus
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | SymbolSelected(Feature_LanguageSupport.DocumentSymbols.symbol);

let update = (~configuration, msg, model) => {
  switch (msg) {
  | KeyboardInput(key) =>
    let model =
      model.focus == FileExplorer
        ? {
          ...model,
          fileExplorer:
            Component_FileExplorer.keyPress(key, model.fileExplorer),
        }
        : {
          ...model,
          symbolOutline: Component_VimTree.keyPress(key, model.symbolOutline),
        };
    (model, Nothing);

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

    let searchText =
        (
          node:
            Component_VimTree.nodeOrLeaf(
              Feature_LanguageSupport.DocumentSymbols.symbol,
              Feature_LanguageSupport.DocumentSymbols.symbol,
            ),
        ) => {
      switch (node) {
      | Leaf({data, _})
      | Node({data, _}) => data.name
      };
    };
    // TODO
    let uniqueId = (symbol: Feature_LanguageSupport.DocumentSymbols.symbol) =>
      symbol.uniqueId;
    (
      {
        ...model,
        symbolOutline:
          Component_VimTree.set(
            ~searchText,
            ~uniqueId,
            symbols,
            model.symbolOutline,
          ),
      },
      Nothing,
    );

  | SymbolOutline(symbolMsg) =>
    let (symbolOutline, outmsg) =
      Component_VimTree.update(symbolMsg, model.symbolOutline);

    let outmsg' =
      switch (outmsg) {
      | Component_VimTree.Nothing
      | Component_VimTree.Expanded(_)
      | Component_VimTree.Collapsed(_) => Nothing

      | Component_VimTree.Selected(symbol) => SymbolSelected(symbol)
      };

    ({...model, symbolOutline}, outmsg');

  | VimWindowNav(navMsg) =>
    let (vimWindowNavigation, outmsg) =
      Component_VimWindows.update(navMsg, model.vimWindowNavigation);

    let model' = {...model, vimWindowNavigation};
    switch (outmsg) {
    | Component_VimWindows.Nothing => (model', Nothing)
    | Component_VimWindows.FocusLeft => (
        model',
        UnhandledWindowMovement(outmsg),
      )
    | Component_VimWindows.FocusRight => (
        model',
        UnhandledWindowMovement(outmsg),
      )
    | Component_VimWindows.FocusDown =>
      if (model'.focus == FileExplorer) {
        ({...model', focus: Outline}, Nothing);
      } else {
        (model', UnhandledWindowMovement(outmsg));
      }
    | Component_VimWindows.FocusUp =>
      if (model'.focus == Outline) {
        ({...model', focus: FileExplorer}, Nothing);
      } else {
        (model', UnhandledWindowMovement(outmsg));
      }
    | Component_VimWindows.PreviousTab
    | Component_VimWindows.NextTab => (model', Nothing)
    };
  };
};

module View = {
  open Revery.UI;
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

    let foregroundColor = Feature_Theme.Colors.foreground.from(theme);

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
          <View style=Style.[paddingRight(4)]>
            <Oni_Components.SymbolIcon theme symbol={symbolData.kind} />
          </View>
          <Text
            text={symbolData.name}
            style=Style.[color(foregroundColor)]
          />
        </View>
      </Oni_Components.Tooltip>;
    };

    let symbolsEmpty =
      <View style=Style.[margin(16)]>
        <Text
          text="No symbols available for active buffer."
          style=Style.[color(foregroundColor)]
        />
      </View>;

    <View style=Style.[flexDirection(`Column), flexGrow(1)]>
      <View style=Style.[flexGrow(2)]>
        <Component_FileExplorer.View
          isFocused={isFocused && model.focus == FileExplorer}
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
        showCount=false
        title="Outline"
        expanded={model.focus == Outline}
        isFocused={isFocused && model.focus == Outline}
        uiFont=font
        theme
        model={model.symbolOutline}
        render=renderSymbol
        onClick={() => ()}
        dispatch={msg => dispatch(SymbolOutline(msg))}
        empty=symbolsEmpty
      />
    </View>;
  };
};

let sub = (~configuration, model) => {
  Component_FileExplorer.sub(~configuration, model.fileExplorer)
  |> Isolinear.Sub.map(msg => FileExplorer(msg));
};

module Contributions = {
  let commands = (~isFocused, model) => {
    let explorerCommands =
      isFocused && model.focus == FileExplorer
        ? Component_FileExplorer.Contributions.commands(~isFocused)
          |> List.map(Oni_Core.Command.map(msg => FileExplorer(msg)))
        : [];

    let outlineCommands =
      isFocused && model.focus == Outline
        ? Component_VimTree.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => SymbolOutline(msg)))
        : [];

    let vimNavCommands =
      isFocused
        ? Component_VimWindows.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)))
        : [];

    explorerCommands @ vimNavCommands @ outlineCommands;
  };

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;
    let vimNavKeys =
      isFocused
        ? Component_VimWindows.Contributions.contextKeys(
            model.vimWindowNavigation,
          )
        : empty;

    let fileExplorerKeys =
      isFocused && model.focus == FileExplorer
        ? Component_FileExplorer.Contributions.contextKeys(
            ~isFocused,
            model.fileExplorer,
          )
        : empty;

    let symbolOutlineKeys =
      isFocused && model.focus == Outline
        ? Component_VimTree.Contributions.contextKeys(model.symbolOutline)
        : empty;

    [fileExplorerKeys, symbolOutlineKeys, vimNavKeys] |> unionMany;
  };
};
