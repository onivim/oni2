open EditorCoreTypes;
open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;

module ListEx = Utility.ListEx;

module Styles = {
  let searchPane = (~theme: Theme.t) =>
    Style.[
      flexDirection(`Row),
      height(200),
      borderTop(~color=theme.sideBarBackground, ~width=1),
    ];

  let queryPane = (~theme: Theme.t) =>
    Style.[
      width(300),
      borderRight(~color=theme.sideBarBackground, ~width=1),
    ];

  let resultsPane = Style.[flexGrow(1)];

  let row =
    Style.[flexDirection(`Row), alignItems(`Center), marginHorizontal(8)];

  let title = (~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(Colors.white),
      marginVertical(8),
      marginHorizontal(8),
    ];

  let input = (~font: UiFont.t) =>
    Style.[
      border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
      backgroundColor(Color.rgba(0., 0., 0., 0.3)),
      color(Colors.white),
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      flexGrow(1),
    ];

  let clickable = Style.[cursor(Revery.MouseCursors.pointer)];
};

let matchToLocListItem = (hit: Ripgrep.Match.t) =>
  LocationListItem.{
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

let make = (~theme, ~uiFont, ~editorFont, ~isFocused, ~state: Search.t, ()) => {
  let items =
    state.hits |> ListEx.safeMap(matchToLocListItem) |> Array.of_list;

  <View style={Styles.searchPane(~theme)}>
    <View style={Styles.queryPane(~theme)}>
      <View style=Styles.row>
        <Text style={Styles.title(~font=uiFont)} text="Find in Files" />
      </View>
      <View style=Styles.row>
        <OniInput
          style={Styles.input(~font=uiFont)}
          cursorColor=Colors.gray
          cursorPosition={state.cursorPosition}
          value={state.queryInput}
          placeholder="Search"
          isFocused
          onClick={pos =>
            GlobalContext.current().dispatch(Actions.SearchInputClicked(pos))
          }
        />
      </View>
    </View>
    <View style=Styles.resultsPane>
      <Text
        style={Styles.title(~font=uiFont)}
        text={Printf.sprintf("%n results", List.length(state.hits))}
      />
      <LocationListView theme uiFont editorFont items />
    </View>
  </View>;
};
