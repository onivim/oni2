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

let resetFocus = model => {...model, focus: FindInput};

let initial = {
  findInput: Component_InputText.create(~placeholder="Search"),
  query: "",
  hits: [],
  focus: FindInput,

  vimWindowNavigation: Component_VimWindows.initial,
  resultsTree: Component_VimTree.create(~rowHeight=25),
};

module Configuration = {
  open Config.Schema;
  let searchExclude = setting("search.exclude", list(string), ~default=[]);
  let filesExclude =
    setting(
      "files.exclude",
      list(string),
      ~default=["_esy", ".git", "node_modules"],
    );
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
    | ResultsPane => (
        {
          ...model,
          resultsTree: Component_VimTree.keyPress(key, model.resultsTree),
        },
        None,
      )
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
  };
};

// SUBSCRIPTIONS

module SearchSubscription =
  SearchSubscription.Make({
    type action = msg;
  });

let subscriptions =
    (~config: Oni_Core.Config.resolver, ~workingDirectory, ripgrep, dispatch) => {
  let searchExclude =
    Configuration.searchExclude.get(config)
    |> List.append(Configuration.filesExclude.get(config));

  let search = query => {
    SearchSubscription.create(
      ~id="workspace-search",
      ~query,
      ~searchExclude,
      ~directory=workingDirectory,
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
  let configuration = Configuration.[searchExclude.spec, filesExclude.spec];
};
