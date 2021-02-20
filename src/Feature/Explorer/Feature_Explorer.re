open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Feature.Explorer"));

// MODEL

type focus =
  | FileExplorer
  | Outline;

module ExpandedState = {
  type t =
    | ExplicitlyClosed
    | ExplicitlyOpened
    | ImplicitlyClosed
    | ImplicitlyOpened;

  let explicitToggle =
    fun
    | ExplicitlyClosed => ExplicitlyOpened
    | ExplicitlyOpened => ExplicitlyClosed
    | ImplicitlyClosed => ExplicitlyOpened
    | ImplicitlyOpened => ExplicitlyClosed;

  let implicitlyOpen =
    fun
    | ExplicitlyClosed => ExplicitlyClosed
    | ExplicitlyOpened => ExplicitlyOpened
    | ImplicitlyClosed => ImplicitlyOpened
    | ImplicitlyOpened => ImplicitlyOpened;

  let implicitlyClose =
    fun
    | ExplicitlyClosed => ExplicitlyClosed
    | ExplicitlyOpened => ExplicitlyOpened
    | ImplicitlyClosed => ImplicitlyClosed
    | ImplicitlyOpened => ImplicitlyClosed;

  let isOpen =
    fun
    | ExplicitlyClosed => false
    | ExplicitlyOpened => true
    | ImplicitlyClosed => false
    | ImplicitlyOpened => true;
};

type model = {
  focus,
  isFileExplorerExpanded: ExpandedState.t,
  fileExplorer: option(Component_FileExplorer.model),
  isSymbolOutlineExpanded: ExpandedState.t,
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
  | VimWindowNav(Component_VimWindows.msg)
  | FileExplorerAccordionClicked
  | SymbolOutlineAccordionClicked
  | OpenFolderClicked;

module Msg = {
  let keyPressed = key => KeyboardInput(key);
  let activeFileChanged = path =>
    FileExplorer(Component_FileExplorer.Msg.activeFileChanged(path));
};

let initial = (~rootPath) => {
  focus: FileExplorer,
  isFileExplorerExpanded: ExpandedState.ImplicitlyOpened,
  isSymbolOutlineExpanded: ExpandedState.ImplicitlyClosed,
  fileExplorer:
    rootPath
    |> Option.map(rootPath => Component_FileExplorer.initial(~rootPath)),
  symbolOutline: Component_VimTree.create(~rowHeight=20),
  vimWindowNavigation: Component_VimWindows.initial,
};

let focusOutline = model => {
  ...model,
  focus: Outline,
  isSymbolOutlineExpanded:
    ExpandedState.implicitlyOpen(model.isSymbolOutlineExpanded),
};

let setRoot = (~rootPath, model) => {
  ...model,
  fileExplorer:
    rootPath
    |> Option.map(rootPath => Component_FileExplorer.initial(~rootPath)),
};

let root = ({fileExplorer, _}) => {
  fileExplorer |> Option.map(Component_FileExplorer.root);
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | PreviewFile(string)
  | GrabFocus
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | SymbolSelected(Feature_LanguageSupport.DocumentSymbols.symbol)
  | PickFolder;

let update = (~config, ~configuration, msg, model) => {
  switch (msg) {
  | KeyboardInput(key) =>
    if (model.focus == FileExplorer) {
      if (model.fileExplorer == None) {
        let lowerKey = String.lowercase_ascii(key);
        if (lowerKey == "<cr>" || lowerKey == "<space>") {
          (model, PickFolder);
        } else {
          (model, Nothing);
        };
      } else {
        (
          {
            ...model,
            fileExplorer:
              model.fileExplorer
              |> Option.map(explorer =>
                   Component_FileExplorer.keyPress(key, explorer)
                 ),
          },
          Nothing,
        );
      };
    } else {
      (
        {
          ...model,
          symbolOutline: Component_VimTree.keyPress(key, model.symbolOutline),
        },
        Nothing,
      );
    }

  | FileExplorer(fileExplorerMsg) =>
    model.fileExplorer
    |> Option.map(fileExplorer => {
         let (fileExplorer, outmsg) =
           Component_FileExplorer.update(
             ~config,
             ~configuration,
             fileExplorerMsg,
             fileExplorer,
           );

         let outmsg' =
           switch (outmsg) {
           | Component_FileExplorer.Nothing => Nothing
           | Component_FileExplorer.Effect(eff) =>
             Effect(eff |> Isolinear.Effect.map(msg => FileExplorer(msg)))
           | Component_FileExplorer.OpenFile(path) => OpenFile(path)
           | Component_FileExplorer.PreviewFile(filePath) =>
             PreviewFile(filePath)
           | GrabFocus => GrabFocus
           };

         ({...model, fileExplorer: Some(fileExplorer)}, outmsg');
       })
    |> Option.value(~default=(model, Nothing))

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
      | Component_VimTree.Touched(symbol) => SymbolSelected(symbol)
      | Component_VimTree.Selected(_) => Nothing
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
        (
          {
            ...model',
            focus: Outline,
            isSymbolOutlineExpanded:
              ExpandedState.implicitlyOpen(model.isSymbolOutlineExpanded),
          },
          Nothing,
        );
      } else {
        (model', UnhandledWindowMovement(outmsg));
      }
    | Component_VimWindows.FocusUp =>
      if (model'.focus == Outline) {
        (
          {
            ...model',
            focus: FileExplorer,
            isSymbolOutlineExpanded:
              ExpandedState.implicitlyClose(model.isSymbolOutlineExpanded),
            isFileExplorerExpanded:
              ExpandedState.implicitlyOpen(model.isFileExplorerExpanded),
          },
          Nothing,
        );
      } else {
        (model', UnhandledWindowMovement(outmsg));
      }
    | Component_VimWindows.PreviousTab
    | Component_VimWindows.NextTab => (model', Nothing)
    };

  | FileExplorerAccordionClicked => (
      {
        ...model,
        isFileExplorerExpanded:
          ExpandedState.explicitToggle(model.isFileExplorerExpanded),
      },
      Nothing,
    )

  | SymbolOutlineAccordionClicked => (
      {
        ...model,
        isSymbolOutlineExpanded:
          ExpandedState.explicitToggle(model.isSymbolOutlineExpanded),
      },
      Nothing,
    )

  | OpenFolderClicked => (model, PickFolder)
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
                  ~font: UiFont.t,
                  ~editorFont: Oni_Core.Font.t,
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
          <View style=Style.[flexGrow(0), flexShrink(0), paddingRight(4)]>
            <Oni_Components.SymbolIcon theme symbol={symbolData.kind} />
          </View>
          <View style=Style.[flexGrow(1), flexShrink(1)]>
            <Text
              text={symbolData.name}
              style=Style.[
                color(foregroundColor),
                textWrap(Revery.TextWrapping.NoWrap),
              ]
              fontFamily={font.family}
              fontSize={font.size}
            />
          </View>
        </View>
      </Oni_Components.Tooltip>;
    };

    let symbolsEmpty =
      <View style=Style.[margin(16)]>
        <Text
          text="No symbols available for active buffer."
          style=Style.[color(foregroundColor)]
          fontFamily={font.family}
          fontSize={font.size}
        />
      </View>;

    let explorerComponent =
      switch (model.fileExplorer) {
      | None =>
        <Component_Accordion.Common
          title="No folder opened"
          expanded={ExpandedState.isOpen(model.isFileExplorerExpanded)}
          count=0
          showCount=false
          isFocused={isFocused && model.focus == FileExplorer}
          theme
          uiFont=font
          onClick={() => dispatch(FileExplorerAccordionClicked)}
          contents={
            <View style=[Style.flexDirection(`Column)]>
              <View
                style=[
                  Style.padding(8),
                  Style.marginLeft(8),
                  Style.marginTop(8),
                ]>
                <Text
                  style=Style.[color(foregroundColor)]
                  text="You have not yet opened a folder."
                  fontFamily={font.family}
                  fontSize={font.size}
                />
              </View>
              <View style=[Style.paddingHorizontal(8)]>
                <Oni_Components.Button
                  label="Open Folder"
                  theme
                  font
                  onClick={() => dispatch(OpenFolderClicked)}
                />
              </View>
              <View
                style=Style.[
                  marginTop(8),
                  justifyContent(`Center),
                  flexDirection(`Row),
                ]>
                <Text
                  style=Style.[color(foregroundColor)]
                  text="(or,  "
                  fontFamily={font.family}
                  fontSize={font.size}
                />
                <View
                  style=[
                    Style.backgroundColor(
                      Revery.Color.rgba(0., 0., 0., 0.2),
                    ),
                    Style.marginTop(-2),
                  ]>
                  <Text
                    style=Style.[color(foregroundColor)]
                    text=":cd"
                    fontFamily={editorFont.fontFamily}
                    fontSize={editorFont.fontSize}
                  />
                </View>
                <Text
                  style=Style.[color(foregroundColor)]
                  text=" )"
                  fontFamily={font.family}
                  fontSize={font.size}
                />
              </View>
            </View>
          }
        />

      | Some(explorer) =>
        <Component_FileExplorer.View
          isFocused={isFocused && model.focus == FileExplorer}
          expanded={ExpandedState.isOpen(model.isFileExplorerExpanded)}
          iconTheme
          languageInfo
          decorations
          model=explorer
          theme
          font
          onRootClicked={() => dispatch(FileExplorerAccordionClicked)}
          dispatch={msg => dispatch(FileExplorer(msg))}
        />
      };

    <View style=Style.[flexDirection(`Column), flexGrow(1)]>
      explorerComponent
      <Component_Accordion.VimTree
        showCount=false
        title="Outline"
        expanded={ExpandedState.isOpen(model.isSymbolOutlineExpanded)}
        isFocused={isFocused && model.focus == Outline}
        uiFont=font
        theme
        model={model.symbolOutline}
        render=renderSymbol
        onClick={() => dispatch(SymbolOutlineAccordionClicked)}
        dispatch={msg => dispatch(SymbolOutline(msg))}
        empty=symbolsEmpty
      />
    </View>;
  };
};

let sub = (~configuration, model) => {
  model.fileExplorer
  |> Option.map(explorer => {
       Component_FileExplorer.sub(~configuration, explorer)
       |> Isolinear.Sub.map(msg => FileExplorer(msg))
     })
  |> Option.value(~default=Isolinear.Sub.none);
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
        ? model.fileExplorer
          |> Option.map(explorer =>
               Component_FileExplorer.Contributions.contextKeys(
                 ~isFocused,
                 explorer,
               )
             )
          |> Option.value(~default=empty)
        : empty;

    let symbolOutlineKeys =
      isFocused && model.focus == Outline
        ? Component_VimTree.Contributions.contextKeys(model.symbolOutline)
        : empty;

    [fileExplorerKeys, symbolOutlineKeys, vimNavKeys] |> unionMany;
  };
};
