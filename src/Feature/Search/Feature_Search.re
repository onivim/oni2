open EditorCoreTypes;
open Oni_Core;
open Utility;
open Oni_Components;

// MODEL

type model = {
  findInput: Feature_InputText.model,
  query: string,
  hits: list(Ripgrep.Match.t),
};

let initial = {
  findInput: Feature_InputText.create(~placeholder="Search"),
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
  | FindInput(Feature_InputText.msg);

type outmsg =
  | Focus;

let update = (model, msg) => {
  switch (msg) {
  | Input(key) =>
    let model =
      switch (key) {
      | "<CR>" =>
        let findInputValue = model.findInput |> Feature_InputText.value;
        if (model.query == findInputValue) {
          model; // Do nothing if the query hasn't changed
        } else {
          {...model, query: findInputValue, hits: []};
        };

      | _ =>
        let findInput = Feature_InputText.handleInput(~key, model.findInput);
        {...model, findInput};
      };

    (model, None);

  | Pasted(text) =>
    let findInput = Feature_InputText.paste(~text, model.findInput);
    ({...model, findInput}, None);

  | FindInput(msg) => (
      {...model, findInput: Feature_InputText.update(msg, model.findInput)},
      Some(Focus),
    )

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

  let pane = [flexGrow(1), flexDirection(`Row)];

  let queryPane = (~theme) => [
    width(300),
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

  let input = [flexGrow(1)];
};

let matchToLocListItem = (hit: Ripgrep.Match.t) =>
  LocationList.{
    file: hit.file,
    location:
      Location.{
        line: Index.fromOneBased(hit.lineNumber),
        column: Index.fromZeroBased(hit.charStart),
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
        <Feature_InputText.View
          style=Styles.input
          model={model.findInput}
          isFocused
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          dispatch={msg => dispatch(FindInput(msg))}
          theme
        />
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
