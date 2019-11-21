open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
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

  let result = (~theme: Theme.t, ~isHovered) =>
    Style.[
      flexDirection(`Row),
      overflow(`Hidden),
      paddingVertical(4),
      paddingHorizontal(8),
      backgroundColor(
        isHovered ? theme.menuSelectionBackground : Colors.transparentWhite,
      ),
    ];

  let fileText = (~font: Types.UiFont.t, ~theme: Theme.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(theme.editorActiveLineNumberForeground),
    ];

  let lineText = (~font: Types.UiFont.t, ~theme: Theme.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(font.fontSize),
      color(theme.editorForeground),
    ];

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

let%component make =
              (~theme: Theme.t, ~font: Types.UiFont.t, ~state: Search.t, ()) => {
  let%hook (hovered, setHovered) = Hooks.state(-1);

  let result = (i, match: Ripgrep.Match.t) => {
    let onMouseOver = _ => setHovered(_ => i);
    let onMouseOut = _ => setHovered(hovered => i == hovered ? (-1) : hovered);

    let directory = Rench.Environment.getWorkingDirectory();
    let re = Str.regexp_string(directory ++ Filename.dir_sep);
    let getDisplayPath = fullPath => Str.replace_first(re, "", fullPath);

    <Clickable
      style=Styles.clickable
      onClick={() =>
        GlobalContext.current().dispatch(SearchSelectResult(match))
      }>
      <View
        style={Styles.result(~theme, ~isHovered=hovered == i)}
        onMouseOver
        onMouseOut>
        <Text
          style={Styles.fileText(~font, ~theme)}
          text={Printf.sprintf(
            "%s:%n - ",
            getDisplayPath(match.file),
            match.lineNumber,
          )}
        />
        <Text
          style={Styles.lineText(~font, ~theme)}
          text={String.trim(match.text)}
        />
      </View>
    </Clickable>;
  };

  let results = state.hits |> Array.of_list;
  let renderResult = i => result(i, results[i]);

  <View style={Styles.searchPane(~theme)}>
    <View style={Styles.queryPane(~theme)}>
      <View style=Styles.row>
        <Text style={Styles.title(~font)} text="Find in Files" />
      </View>
      <View style=Styles.row>
        <OniInput
          style={Styles.input(~font)}
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
        style={Styles.title(~font)}
        text={Printf.sprintf("%n results", List.length(state.hits))}
      />
      <FlatList
        rowHeight=20
        count={Array.length(results)}
        focused=None
        render=renderResult
      />
    </View>
  </View>;
};