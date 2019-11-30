open Revery;
open Revery.UI;
open Oni_Core;
open Types;
open Oni_Model;

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

  let title = (~font: Types.UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(Colors.white),
      marginVertical(8),
      marginHorizontal(8),
    ];

  let input = (~font: Types.UiFont.t) =>
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
      Position.create(
        Index.ofInt1(hit.lineNumber),
        Index.ofInt0(hit.charStart),
      ),
    text: hit.text,
    highlight:
      Some((Index.ofInt1(hit.charStart), Index.ofInt1(hit.charEnd))),
  };

let make = (~theme, ~uiFont, ~editorFont, ~state: Search.t, ()) => {
  let items = state.hits |> List.map(matchToLocListItem) |> Array.of_list;

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
          onKeyDown={(event: NodeEvents.keyEventParams) =>
            if (event.keycode == 13) {
              GlobalContext.current().dispatch(Actions.SearchStart);
            }
          }
          onChange={(text, pos) =>
            GlobalContext.current().dispatch(Actions.SearchInput(text, pos))
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
