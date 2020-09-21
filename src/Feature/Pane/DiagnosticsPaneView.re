/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open Oni_Core;

open Revery.UI;
open Oni_Components;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let pane = [flexGrow(1), flexDirection(`Row)];

  let noResultsContainer = [
    flexGrow(1),
    alignItems(`Center),
    justifyContent(`Center),
  ];

  let title = (~theme) => [
    color(Colors.PanelTitle.activeForeground.from(theme)),
    margin(8),
  ];
};

let make =
    (
      ~diagnosticsList: Component_VimList.model(LocationListItem.t),
      ~theme,
      ~uiFont: UiFont.t,
      ~editorFont,
      ~workingDirectory,
      ~dispatch: Component_VimList.msg => unit,
      (),
    ) => {
  let innerElement =
    if (Component_VimList.count(diagnosticsList) == 0) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          text="No problems, yet!"
        />
      </View>;
    } else {
      <Component_VimList.View
        theme
        model=diagnosticsList
        dispatch
        render={(~availableWidth, ~index as _, ~hovered, ~focused, item) =>
          <LocationListItem.View
            width=availableWidth
            theme
            uiFont
            editorFont
            isHovered={hovered || focused}
            item
            workingDirectory
          />
        }
      />;
    };

  <View style=Styles.pane> innerElement </View>;
};
