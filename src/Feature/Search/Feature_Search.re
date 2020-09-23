open EditorCoreTypes;
open Oni_Core;
open Oni_Components;

// MODEL

type focus =
  | FindInput
  | ResultsPane;

type model = {
  findInput: Component_InputText.model,
  query: string,
  hits: list(Ripgrep.Match.t),
  focus,
  vimWindowNavigation: Component_VimWindows.model,
  resultsTree: Component_VimTree.model(string, LocationListItem.t),
};

let initial = {
  findInput: Component_InputText.create(~placeholder="Search"),
  query: "",
  hits: [],
  focus: FindInput,

  vimWindowNavigation: Component_VimWindows.initial,
  resultsTree: Component_VimTree.create(~rowHeight=25),
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
  | ResultsList(Component_VimTree.msg);

module Msg = {
  let input = str => Input(str);
  let pasted = str => Pasted(str);
};

type outmsg =
  | OpenFile({
      filePath: string,
      location: CharacterPosition.t,
    })
  | Focus
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

let update = (model, msg) => {
  switch (msg) {
  | Input(key) =>
    switch (model.focus) {
    | FindInput =>
      let model =
        switch (key) {
        | "<CR>" =>
          let findInputValue = model.findInput |> Component_InputText.value;
          if (model.query == findInputValue) {
            model; // Do nothing if the query hasn't changed
          } else {
            {...model, query: findInputValue} |> setHits([]);
          };

        | _ =>
          let findInput =
            Component_InputText.handleInput(~key, model.findInput);
          {...model, findInput};
        };

      (model, None);
    | ResultsPane =>
      // TODO: Vim Navigable List!
      (model, None)
    }

  | Pasted(text) =>
    switch (model.focus) {
    | FindInput =>
      let findInput = Component_InputText.paste(~text, model.findInput);
      ({...model, findInput}, None);
    | ResultsPane =>
      // Paste is a no-op in search pane
      (model, None)
    }

  | FindInput(msg) =>
    let (findInput', inputOutmsg) =
      Component_InputText.update(msg, model.findInput);
    let outmsg =
      switch (inputOutmsg) {
      | Component_InputText.Nothing => None
      | Component_InputText.Focus => Some(Focus)
      };
    ({...model, findInput: findInput'}, outmsg);

  | Update(items) => (model |> setHits(model.hits @ items), None)

  | VimWindowNav(navMsg) =>
    let (windowNav, outmsg) =
      Component_VimWindows.update(navMsg, model.vimWindowNavigation);

    let model' = {...model, vimWindowNavigation: windowNav};
    switch (outmsg) {
    | Nothing => (model', None)
    | FocusLeft => (model', Some(UnhandledWindowMovement(outmsg)))
    | FocusRight => (model', Some(UnhandledWindowMovement(outmsg)))
    | FocusDown =>
      if (model'.focus == FindInput) {
        ({...model', focus: ResultsPane}, None);
      } else {
        (model', Some(UnhandledWindowMovement(outmsg)));
      }
    | FocusUp =>
      if (model'.focus == ResultsPane) {
        ({...model', focus: FindInput}, None);
      } else {
        (model', Some(UnhandledWindowMovement(outmsg)));
      }
    // TODO: What should tabs do for search? Toggle sidebar panes?
    | PreviousTab
    | NextTab => (model', None)
    };

  | ResultsList(listMsg) =>
    let (resultsTree, outmsg) =
      Component_VimTree.update(listMsg, model.resultsTree);

    let eff =
      Component_VimTree.(
        switch (outmsg) {
        | Nothing => None
        | Selected(item) =>
          Some(OpenFile({filePath: item.file, location: item.location}))
        // TODO
        | Collapsed(_) => None
        | Expanded(_) => None
        }
      );

    ({...model, resultsTree}, eff);

  | Complete => (model, None)

  | SearchError(_) => (model, None)
  };
};

// SUBSCRIPTIONS

module SearchSubscription =
  SearchSubscription.Make({
    type action = msg;
  });

let subscriptions = (ripgrep, dispatch) => {
  let search = query => {
    let directory = Rench.Environment.getWorkingDirectory();

    SearchSubscription.create(
      ~id="workspace-search",
      ~query,
      ~directory,
      ~ripgrep,
      ~onUpdate=items => dispatch(Update(items)),
      ~onCompleted=() => Complete,
      ~onError=msg => SearchError(msg),
    );
  };

  model => {
    switch (model) {
    | {query: "", _} => []
    | {query, _} => [search(query)]
    };
  };
};

// VIEW

open Revery.UI;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let pane = [flexGrow(1), flexDirection(`Column)];

  let queryPane = (~theme) => [
    borderRight(~color=Colors.Panel.border.from(theme), ~width=1),
  ];

  let resultsPane = (~isFocused, ~theme) => {
    let focusColor =
      isFocused
        ? Colors.focusBorder.from(theme) : Revery.Colors.transparentWhite;
    [flexGrow(1), border(~color=focusColor, ~width=1)];
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
};

let make =
    (
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
    <View style={Styles.queryPane(~theme)}>
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
      </View>
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
        isActive={isFocused && model.focus == ResultsPane}
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
  open WhenExpr.ContextKeys.Schema;

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

  let contextKeys = (~isFocused) => {
    let inputTextKeys =
      isFocused ? Component_InputText.Contributions.contextKeys : [];
    let vimNavKeys =
      isFocused ? Component_VimWindows.Contributions.contextKeys : [];

    let vimListKeys =
      isFocused ? Component_VimList.Contributions.contextKeys : [];

    [
      inputTextKeys |> fromList |> map(({findInput, _}: model) => findInput),
      vimNavKeys
      |> fromList
      |> map(({vimWindowNavigation, _}: model) => vimWindowNavigation),
      vimListKeys |> fromList |> map(_ => ()),
    ]
    |> unionMany;
  };
};
