/*
 * Diagnostics.re
 *
 * This module is responsible for tracking the state of 'diagnostics'
 * (usually errors or warnings) that we render in the buffer view
 * or minimap.
 */

open Oni_Core;

open Revery.UI;

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
      ~notificationsList:
         Component_VimList.model(Feature_Notification.notification),
      ~theme,
      ~uiFont: UiFont.t,
      ~dispatch: Component_VimList.msg => unit,
      ~onDismiss: Feature_Notification.notification => unit,
      (),
    ) => {
  let innerElement =
    if (Component_VimList.count(notificationsList) == 0) {
      <View style=Styles.noResultsContainer>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          text="No notifications."
        />
      </View>;
    } else {
      <Component_VimList.View
        font=uiFont
        isActive=isFocused
        focusedIndex=None
        theme
        model=notificationsList
        dispatch
        render={(
          ~availableWidth as _,
          ~index as _,
          ~hovered as _,
          ~selected as _,
          item,
        ) =>
          <Feature_Notification.View.Item
            notification=item
            font=uiFont
            theme
            onDismiss={() => onDismiss(item)}
          />
        }
      />;
    };

  <View style=Styles.pane> innerElement </View>;
};
