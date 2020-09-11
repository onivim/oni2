open EditorCoreTypes;
open Oni_Core;
open Utility;
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
};

let initial = {
  findInput: Component_InputText.create(~placeholder="Search"),
  query: "",
  hits: [],
  focus: FindInput,

  vimWindowNavigation: Component_VimWindows.initial,
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
  | VimWindowNav(Component_VimWindows.msg);

module Msg = {
  let input = str => Input(str);
  let pasted = str => Pasted(str);
};

type outmsg =
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
            {...model, query: findInputValue, hits: []};
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

  | Update(items) => ({...model, hits: model.hits @ items}, None)

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
    };

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
    let common = [flexGrow(1)];

    let focused = [border(~color=Colors.focusBorder.from(theme), ~width=1)];

    isFocused ? common @ focused : common;
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

let matchToLocListItem = (hit: Ripgrep.Match.t) =>
  LocationList.{
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

let make =
    (
      ~theme,
      ~uiFont: UiFont.t,
      ~editorFont,
      ~isFocused,
      ~model,
      ~onSelectResult,
      ~dispatch,
      (),
    ) => {
  let items =
    model.hits |> ListEx.safeMap(matchToLocListItem) |> Array.of_list;

  let onSelectItem = (item: LocationList.item) =>
    onSelectResult(item.file, item.location);

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
        ~isFocused={isFocused && model.focus == ResultsPane},
        ~theme,
      )}>
      <Text
        style={Styles.title(~theme)}
        fontFamily={uiFont.family}
        fontSize={uiFont.size}
        text={Printf.sprintf("%n results", List.length(model.hits))}
      />
      <LocationList theme uiFont editorFont items onSelectItem />
    </View>
  </View>;
};

module Contributions = {
  open WhenExpr.ContextKeys.Schema;

  let commands = (~isFocused) => {
    !isFocused
      ? []
      : Component_VimWindows.Contributions.commands
        |> List.map(Oni_Core.Command.map(msg => VimWindowNav(msg)));
  };

  let contextKeys = (~isFocused) => {
    let inputTextKeys =
      isFocused ? Component_InputText.Contributions.contextKeys : [];
    let vimNavKeys =
      isFocused ? Component_VimWindows.Contributions.contextKeys : [];

    [
      inputTextKeys |> fromList |> map(({findInput, _}: model) => findInput),
      vimNavKeys
      |> fromList
      |> map(({vimWindowNavigation, _}: model) => vimWindowNavigation),
    ]
    |> unionMany;
  };
};
