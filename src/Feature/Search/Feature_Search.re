open EditorCoreTypes;
open Oni_Core;
open Utility;
open Oni_Components;

// MODEL

type model = {
  queryInput: string,
  query: string,
  selection: Selection.t,
  hits: list(Ripgrep.Match.t),
};

let initial = {
  queryInput: "",
  query: "",
  selection: Selection.initial,
  hits: [],
};

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | Input(string)
  | InputClicked(Selection.t)
  | Update([@opaque] list(Ripgrep.Match.t))
  | Complete;

type outmsg =
  | Focus;

let update = (model, msg) => {
  switch (msg) {
  | Input(key) =>
    let {queryInput, selection, _} = model;

    let model =
      switch (key) {
      | "<CR>" =>
        if (model.query == model.queryInput) {
          model; // Do nothing if the query hasn't changed
        } else {
          {...model, query: model.queryInput, hits: []};
        }

      | _ =>
        let (queryInput, selection) =
          InputModel.handleInput(~text=queryInput, ~selection, key);
        {...model, queryInput, selection};
      };

    (model, None);

  | InputClicked(selection) => ({...model, selection}, Some(Focus))

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

  let title = (~font: UiFont.t, ~theme) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    color(Colors.PanelTitle.activeForeground.from(theme)),
    marginVertical(8),
    marginHorizontal(8),
  ];

  let input = (~font: UiFont.t) => [
    fontFamily(font.fontFile),
    fontSize(font.fontSize),
    flexGrow(1),
  ];
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
      ~uiFont,
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
          style={Styles.title(~font=uiFont, ~theme)}
          text="Find in Files"
        />
      </View>
      <View style=Styles.row>
        <Input
          style={Styles.input(~font=uiFont)}
          selection={model.selection}
          value={model.queryInput}
          placeholder="Search"
          isFocused
          onClick={selection => dispatch(InputClicked(selection))}
          theme
        />
      </View>
    </View>
    <View style=Styles.resultsPane>
      <Text
        style={Styles.title(~font=uiFont, ~theme)}
        text={Printf.sprintf("%n results", List.length(model.hits))}
      />
      <LocationList theme uiFont editorFont items onSelectItem />
    </View>
  </View>;
};
