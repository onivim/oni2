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
      ~isFocused: bool,
      ~diagnosticsList: Component_VimTree.model(string, LocationListItem.t),
      ~theme,
      ~iconTheme,
      ~languageInfo,
      ~uiFont: UiFont.t,
      ~workingDirectory,
      ~dispatch: Component_VimTree.msg => unit,
      (),
    ) => {
  let innerElement =
    if (Component_VimTree.count(diagnosticsList) == 0) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          text="No problems, yet!"
        />
      </View>;
    } else {
      <Component_VimTree.View
        isActive=isFocused
        theme
        model=diagnosticsList
        dispatch
        render={(
          ~availableWidth,
          ~index as _,
          ~hovered as _,
          ~focused as _,
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
              showPosition=true
              width=availableWidth
              theme
              uiFont
              item=data
            />
          }
        }
      />;
    };

  <View style=Styles.pane> innerElement </View>;
};
