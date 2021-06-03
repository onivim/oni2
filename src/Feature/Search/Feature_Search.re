open EditorCoreTypes;
open Oni_Core;
open Oni_Components;

module Constants = {
  let optionIconSize = 14.;
};

// MODEL

type focus =
  | FindInput
  | ResultsPane
  | ToggleRegexButton
  | ToggleCaseSensitiveButton
  | ToggleOptionsButton;

type model = {
  findInput: Component_InputText.model,
  enableRegex: bool,
  caseSensitive: bool,
  optionsVisible: bool,
  query: string,
  searchNonce: int,
  hits: list(Ripgrep.Match.t),
  focus,
  vimWindowNavigation: Component_VimWindows.model,
  resultsTree: Component_VimTree.model(string, LocationListItem.t),
};

let resetFocus = (~query: option(string), model) => {
  switch (query) {
  | None => {...model, focus: FindInput}
  | Some(query) => {
      ...model,
      query,
      focus: FindInput,
      findInput: Component_InputText.set(~text=query, model.findInput),
      hits: query != model.query ? [] : model.hits,
    }
  };
};

let initial = {
  findInput: Component_InputText.create(~placeholder="Search"),
  enableRegex: false,
  caseSensitive: false,
  optionsVisible: false,
  query: "",
  searchNonce: 0,
  hits: [],
  focus: FindInput,

  vimWindowNavigation: Component_VimWindows.initial,
  resultsTree: Component_VimTree.create(~rowHeight=25),
};

module Configuration = {
  open Config.Schema;
  let searchExclude = setting("search.exclude", list(string), ~default=[]);
};

let matchToLocListItem = (hit: Ripgrep.Match.t) =>
  LocationListItem.{
    file: hit.file,
    location:
      CharacterPosition.{
        line: EditorCoreTypes.LineNumber.ofOneBased(hit.lineNumber),
        character: CharacterIndex.ofInt(hit.charStart),
      },
    text: hit.text,
    highlight:
      Some((
        Index.fromZeroBased(hit.charStart),
        Index.fromZeroBased(hit.charEnd),
      )),
  };

let setHits = (hits, model) => {
  ...model,
  hits,
  resultsTree:
    Component_VimTree.set(
      ~uniqueId=path => path,
      ~searchText=
        Component_VimTree.(
          fun
          | Node({data, _}) => data
          | Leaf({data, _}) => LocationListItem.(data.text)
        ),
      hits |> List.map(matchToLocListItem) |> LocationListItem.toTrees,
      model.resultsTree,
    ),
};

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | Input(string)
  | Pasted(string)
  | Update([@opaque] list(Ripgrep.Match.t))
  | Complete
  | SearchError(string)
  | FindInput(Component_InputText.msg)
  | VimWindowNav(Component_VimWindows.msg)
  | ResultsList(Component_VimTree.msg)
  | ToggleRegexButtonClicked
  | ToggleOptionsButtonClicked
  | ToggleCaseSensitiveButtonClicked;

module Msg = {
  let input = str => Input(str);
  let pasted = str => Pasted(str);
};

type outmsg =
  | OpenFile({
      filePath: string,
      location: CharacterPosition.t,
    })
  | PreviewFile({
      filePath: string,
      location: CharacterPosition.t,
    })
  | Focus
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

let update = (~previewEnabled, model, msg) => {
  switch (msg) {
  | Input(key) =>
    switch (model.focus) {
    | FindInput =>
      let model =
        switch (key) {
        | "<CR>" =>
          let findInputValue = model.findInput |> Component_InputText.value;
          {
            ...model,
            query: findInputValue,
            searchNonce: model.searchNonce + 1,
          }
          |> setHits([]);

        | _ =>
          let findInput =
            Component_InputText.handleInput(~key, model.findInput);
          {...model, findInput};
        };

      (model, None);
    | ResultsPane => (
        {
          ...model,
          resultsTree: Component_VimTree.keyPress(key, model.resultsTree),
        },
        None,
      )
    | ToggleRegexButton =>
      switch (key) {
      | "<CR>" => ({...model, enableRegex: !model.enableRegex}, None)
      | _ => (model, None)
      }
    | ToggleCaseSensitiveButton =>
      switch (key) {
      | "<CR>" => ({...model, caseSensitive: !model.caseSensitive}, None)
      | _ => (model, None)
      }
    | ToggleOptionsButton =>
      switch (key) {
      | "<CR>" => ({...model, optionsVisible: !model.optionsVisible}, None)
      | _ => (model, None)
      }
    }

  | Pasted(text) =>
    switch (model.focus) {
    | FindInput =>
      let findInput = Component_InputText.paste(~text, model.findInput);
      ({...model, findInput}, None);
    | _ =>
      // Paste is a no-op in search pane and buttons
      (model, None)
    }

  | FindInput(msg) =>
    let (findInput', inputOutmsg) =
      Component_InputText.update(msg, model.findInput);
    let (model', outmsg) =
      switch (inputOutmsg) {
      | Component_InputText.Nothing => (model, None)
      | Component_InputText.Focus => (
          {...model, focus: FindInput},
          Some(Focus),
        )
      };
    ({...model', findInput: findInput'}, outmsg);

  | Update(items) => (model |> setHits(model.hits @ items), None)

  | VimWindowNav(navMsg) =>
    let (windowNav, outmsg) =
      Component_VimWindows.update(navMsg, model.vimWindowNavigation);

    let model' = {...model, vimWindowNavigation: windowNav};
    switch (outmsg) {
    | Nothing => (model', None)
    | FocusLeft =>
      switch (model'.focus) {
      | ToggleOptionsButton => ({...model', focus: FindInput}, None)
      | ToggleCaseSensitiveButton => (
          {...model', focus: ToggleRegexButton},
          None,
        )
      | _ => (model', Some(UnhandledWindowMovement(outmsg)))
      }
    | FocusRight =>
      switch (model'.focus) {
      | FindInput => ({...model', focus: ToggleOptionsButton}, None)
      | ToggleRegexButton => (
          {...model', focus: ToggleCaseSensitiveButton},
          None,
        )
      | _ => (model', Some(UnhandledWindowMovement(outmsg)))
      }
    | FocusDown =>
      switch (model'.focus, model'.optionsVisible) {
      | (FindInput, true)
      | (ToggleOptionsButton, true) => (
          {...model', focus: ToggleRegexButton},
          None,
        )

      | (ToggleRegexButton | ToggleCaseSensitiveButton, _)
      | (FindInput, false)
      | (ToggleOptionsButton, false) => (
          {...model', focus: ResultsPane},
          None,
        )
      | _ => (model', Some(UnhandledWindowMovement(outmsg)))
      }
    | FocusUp =>
      switch (model'.focus, model'.optionsVisible) {
      | (ResultsPane, true) => ({...model', focus: ToggleRegexButton}, None)
      | (ResultsPane, false) => ({...model', focus: FindInput}, None)
      | (ToggleRegexButton | ToggleCaseSensitiveButton, _) => (
          {...model', focus: FindInput},
          None,
        )
      | _ => (model', Some(UnhandledWindowMovement(outmsg)))
      }
    // TODO: What should tabs do for search? Toggle sidebar panes?
    | PreviousTab
    | NextTab => (model', None)
    };

  | ResultsList(listMsg) =>
    let (resultsTree, outmsg) =
      Component_VimTree.update(listMsg, model.resultsTree);

    let eff =
      switch (outmsg) {
      | Component_VimTree.Nothing => None
      | Component_VimTree.Touched(item) =>
        Some(
          previewEnabled
            ? PreviewFile({filePath: item.file, location: item.location})
            : OpenFile({filePath: item.file, location: item.location}),
        )
      | Component_VimTree.Selected(item) =>
        Some(OpenFile({filePath: item.file, location: item.location}))
      // TODO
      | Component_VimTree.SelectedNode(_) => None
      | Component_VimTree.Collapsed(_) => None
      | Component_VimTree.Expanded(_) => None
      };

    ({...model, resultsTree}, eff);

  | Complete => (model, None)

  | SearchError(_) => (model, None)

  | ToggleOptionsButtonClicked => (
      {
        ...model,
        optionsVisible: !model.optionsVisible,
        focus: ToggleOptionsButton,
      },
      None,
    )

  | ToggleRegexButtonClicked => (
      {
        ...model,
        enableRegex: !model.enableRegex,
        searchNonce: model.searchNonce + 1,
        focus: ToggleRegexButton,
      }
      |> setHits([]),
      None,
    )
  | ToggleCaseSensitiveButtonClicked => (
      {
        ...model,
        caseSensitive: !model.caseSensitive,
        searchNonce: model.searchNonce + 1,
        focus: ToggleCaseSensitiveButton,
      }
      |> setHits([]),
      None,
    )
  };
};

// SUBSCRIPTIONS

let sub = (~config, ~workingDirectory, ~setup, model) => {
  let query = model.query;

  if (Utility.StringEx.isEmpty(query)) {
    Isolinear.Sub.none;
  } else {
    let exclude =
      Configuration.searchExclude.get(config)
      |> List.append(
           Feature_Configuration.GlobalConfiguration.Files.exclude.get(
             config,
           ),
         );
    let toMsg =
      fun
      | Service_Ripgrep.Sub.GotMatches(items) => Update(items)
      | Service_Ripgrep.Sub.Completed => Complete
      | Service_Ripgrep.Sub.Error(msg) => SearchError(msg);

    Service_Ripgrep.Sub.findInFiles(
      ~exclude,
      ~directory=workingDirectory,
      ~query=model.query,
      ~uniqueId=string_of_int(model.searchNonce),
      ~setup,
      ~enableRegex=model.enableRegex,
      ~caseSensitive=model.caseSensitive,
      toMsg,
    );
  };
};

// VIEW

open Revery.UI;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let pane = [flexGrow(1), flexDirection(`Column)];

  let focusColor = (~isFocused, ~theme) =>
    isFocused
      ? Colors.focusBorder.from(theme) : Revery.Colors.transparentWhite;

  let resultsPane = (~isFocused, ~theme) => {
    [flexGrow(1), border(~color=focusColor(~isFocused, ~theme), ~width=1)];
  };

  let row = [
    flexDirection(`Row),
    alignItems(`Center),
    marginHorizontal(8),
  ];

  let title = (~theme) => [
    color(Colors.PanelTitle.activeForeground.from(theme)),
    marginVertical(8),
    marginHorizontal(8),
  ];

  let inputContainer = [width(150), flexShrink(0), flexGrow(1)];
  let inputOption = (~isFocused, ~theme) => [
    cursor(Revery.MouseCursors.pointer),
    padding(2),
    border(~color=focusColor(~isFocused, ~theme), ~width=1),
  ];
};

let make =
    (
      ~config,
      ~theme,
      ~uiFont: UiFont.t,
      ~iconTheme,
      ~languageInfo,
      ~isFocused,
      ~model,
      ~dispatch,
      ~workingDirectory,
      (),
    ) => {
  <View style=Styles.pane>
    <View>
      <View style=Styles.row>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          text="Find in Files"
        />
      </View>
      <View style=Styles.row>
        <View style=Styles.inputContainer>
          <Component_InputText.View
            model={model.findInput}
            isFocused={isFocused && model.focus == FindInput}
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            dispatch={msg => dispatch(FindInput(msg))}
            theme
          />
        </View>
        <Tooltip text="Toggle advanced search options">
          <Feature_Sneak.View.Sneakable
            sneakId="search.toggleOptions"
            style={Styles.inputOption(
              ~isFocused=model.focus == ToggleOptionsButton,
              ~theme,
            )}
            onClick={_ => dispatch(ToggleOptionsButtonClicked)}>
            <Codicon
              icon={
                model.optionsVisible ? Codicon.chevronUp : Codicon.chevronDown
              }
              fontSize=10.
              color={Colors.PanelTitle.inactiveForeground.from(theme)}
            />
          </Feature_Sneak.View.Sneakable>
        </Tooltip>
      </View>
      {model.optionsVisible
         ? <View style=Styles.row>
             <Tooltip text="Toggle regular expression syntax">
               <Feature_Sneak.View.Sneakable
                 sneakId="search.toggleRegex"
                 style={Styles.inputOption(
                   ~isFocused=model.focus == ToggleRegexButton,
                   ~theme,
                 )}
                 onClick={_ => dispatch(ToggleRegexButtonClicked)}>
                 <Codicon
                   icon=Codicon.regex
                   fontSize=Constants.optionIconSize
                   color={
                     model.enableRegex
                       ? Colors.PanelTitle.activeForeground.from(theme)
                       : Colors.PanelTitle.inactiveForeground.from(theme)
                   }
                 />
               </Feature_Sneak.View.Sneakable>
             </Tooltip>
             <Tooltip text="Toggle case-sensitivity">
               <Feature_Sneak.View.Sneakable
                 sneakId="search.toggleCaseSensitive"
                 style={Styles.inputOption(
                   ~isFocused=model.focus == ToggleCaseSensitiveButton,
                   ~theme,
                 )}
                 onClick={_ => dispatch(ToggleCaseSensitiveButtonClicked)}>
                 <Codicon
                   icon=Codicon.caseSensitive
                   fontSize=Constants.optionIconSize
                   color={
                     model.caseSensitive
                       ? Colors.PanelTitle.activeForeground.from(theme)
                       : Colors.PanelTitle.inactiveForeground.from(theme)
                   }
                 />
               </Feature_Sneak.View.Sneakable>
             </Tooltip>
           </View>
         : <View />}
    </View>
    <View
      style={Styles.resultsPane(
        ~isFocused=isFocused && model.focus == ResultsPane,
        ~theme,
      )}>
      <Text
        style={Styles.title(~theme)}
        fontFamily={uiFont.family}
        fontSize={uiFont.size}
        text={Printf.sprintf("%n results", List.length(model.hits))}
      />
      <Component_VimTree.View
        config
        isActive={isFocused && model.focus == ResultsPane}
        font=uiFont
        focusedIndex=None
        theme
        model={model.resultsTree}
        dispatch={msg => dispatch(ResultsList(msg))}
        render={(
          ~availableWidth,
          ~index as _,
          ~hovered as _,
          ~selected as _,
          item,
        ) =>
          switch (item) {
          | Component_VimTree.Node({data, _}) =>
            <FileItemView.View
              theme
              uiFont
              iconTheme
              languageInfo
              item=data
              workingDirectory
            />
          | Component_VimTree.Leaf({data, _}) =>
            <LocationListItem.View
              width=availableWidth
              theme
              uiFont
              item=data
            />
          }
        }
      />
    </View>
  </View>;
};

module Contributions = {
  let commands = (~isFocused) => {
    !isFocused
      ? []
      : (
          Component_VimWindows.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)))
        )
        @ (
          Component_VimTree.Contributions.commands
          |> List.map(Oni_Core.Command.map(msg => ResultsList(msg)))
        );
  };

  let contextKeys = (~isFocused, model) => {
    open WhenExpr.ContextKeys;
    let inputTextKeys =
      isFocused && model.focus == FindInput
        ? Component_InputText.Contributions.contextKeys(model.findInput)
        : empty;
    let vimNavKeys =
      isFocused
        ? Component_VimWindows.Contributions.contextKeys(
            model.vimWindowNavigation,
          )
        : empty;

    let vimTreeKeys =
      isFocused && model.focus == ResultsPane
        ? Component_VimTree.Contributions.contextKeys(model.resultsTree)
        : empty;

    [inputTextKeys, vimNavKeys, vimTreeKeys] |> unionMany;
  };
  let configuration = Configuration.[searchExclude.spec];
};
