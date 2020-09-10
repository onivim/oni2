open EditorCoreTypes;
open Oni_Core;
open Utility;
open Oni_Components;

// MODEL

type model = {
  findInput: Component_InputText.model,
  query: string,
  hits: list(Ripgrep.Match.t),
};

let initial = {
  findInput: Component_InputText.create(~placeholder="Search"),
  query: "",
  hits: [],
};

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | Input(string)
  | Pasted(string)
  | Update([@opaque] list(Ripgrep.Match.t))
  | Complete
  | SearchError(string)
  | FindInput(Component_InputText.msg);

type outmsg =
  | Focus;

let update = (model, msg) => {
  switch (msg) {
  | Input(key) =>
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

  | Pasted(text) =>
    let findInput = Component_InputText.paste(~text, model.findInput);
    ({...model, findInput}, None);

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

  | _ => (model, None)
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

  let resultsPane = [flexGrow(1)];

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
            isFocused
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            dispatch={msg => dispatch(FindInput(msg))}
            theme
          />
        </View>
      </View>
    </View>
    <View style=Styles.resultsPane>
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

  let contextKeys = (~isFocused) => {
    let keys = isFocused ? Component_InputText.Contributions.contextKeys : [];

    [keys |> fromList |> map(({findInput, _}: model) => findInput)]
    |> unionMany;
  };
};
